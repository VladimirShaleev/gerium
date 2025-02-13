#include "Model.hpp"

struct Cache {
    std::set<const aiMesh*> meshes;
    std::vector<std::tuple<size_t, size_t>> nodeNames;
};

static std::tuple<size_t, size_t> appendString(Model& model, const char* name, size_t length) {
    auto nameIndex = model.strPool.size();
    model.strPool.resize(nameIndex + length + 1);
    memcpy(model.strPool.data() + nameIndex, name, length);
    return { nameIndex, length };
}

static void appendMaterial(Cache& cache, Model& model, const aiScene* sc, const aiMesh* mesh) {
    if (sc->HasMaterials()) {
        const auto& material = sc->mMaterials[mesh->mMaterialIndex];
        const auto& name     = hashed_string_owner(material->GetName().C_Str());

        auto it = std::find_if(model.materials.cbegin(), model.materials.cend(), [&name](const auto& item) {
            return item.name == name;
        });

        if (it != model.materials.cend()) {
            model.meshes.back().materialIndex = (gerium_uint32_t) std::distance(model.materials.cbegin(), it);
            return;
        }

        const auto basePath = std::filesystem::path("models") / "textures";

        auto getTexture = [&](aiTextureType type) -> hashed_string_owner {
            if (material->GetTextureCount(type)) {
                aiString name;
                material->Get(AI_MATKEY_TEXTURE(type, 0), name);
                auto nameStr = (basePath / name.C_Str()).string();
                return hashed_string_owner{ nameStr.c_str(), nameStr.length() };
            }
            return {};
        };

        model.materials.push_back({});
        auto& pbr = model.materials.back();

        pbr.name                     = std::move(name);
        pbr.baseColorTexture         = getTexture(aiTextureType_BASE_COLOR);
        pbr.metallicRoughnessTexture = getTexture(aiTextureType_METALNESS);
        pbr.normalTexture            = getTexture(aiTextureType_NORMALS);
        pbr.occlusionTexture         = getTexture(aiTextureType_LIGHTMAP);
        pbr.emissiveTexture          = getTexture(aiTextureType_EMISSIVE);

        material->Get(AI_MATKEY_BASE_COLOR, pbr.baseColorFactor);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, pbr.emissiveFactor);
        material->Get(AI_MATKEY_METALLIC_FACTOR, pbr.metallicFactor);
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, pbr.roughnessFactor);
        material->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), pbr.occlusionStrength);
        material->Get(AI_MATKEY_GLTF_ALPHACUTOFF, pbr.alphaCutoff);

        aiString alphaMode{};
        material->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
        auto alphaModeStr = std::string(alphaMode.C_Str(), alphaMode.length);

        if (alphaModeStr == "MASK") {
            pbr.flags |= MaterialFlags::AlphaMask;
        } else if (alphaModeStr == "BLEND") {
            pbr.flags |= MaterialFlags::Transparent;
        }

        bool doubleSided;
        material->Get(AI_MATKEY_TWOSIDED, doubleSided);
        if (doubleSided) {
            pbr.flags |= MaterialFlags::DoubleSided;
        }
    }
}

static void appendMesh(Cluster& cluster, Cache& cache, Model& model, const aiScene* sc, const aiMesh* mesh) {
    auto indexCount = mesh->mNumFaces * 3;
    std::vector<uint32_t> remap(indexCount);
    std::vector<uint32_t> indicesOrigin(indexCount);
    std::vector<VertexNonCompressed> verticesOrigin(mesh->mNumVertices);

    for (int v = 0; v < mesh->mNumVertices; ++v) {
        const auto vertex    = mesh->mVertices[v];
        const auto normal    = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
        const auto tangent   = glm::vec3(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
        const auto bitangent = glm::vec3(mesh->mBitangents[v].x, mesh->mBitangents[v].y, mesh->mBitangents[v].z);
        const auto tangentW  = (glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f) ? -1.0f : 1.0f;
        const auto texcoord  = mesh->mTextureCoords[0][v];

        verticesOrigin[v].px = vertex.x;
        verticesOrigin[v].py = vertex.y;
        verticesOrigin[v].pz = vertex.z;
        verticesOrigin[v].nx = normal.x;
        verticesOrigin[v].ny = normal.y;
        verticesOrigin[v].nz = normal.z;
        verticesOrigin[v].tx = tangent.x;
        verticesOrigin[v].ty = tangent.y;
        verticesOrigin[v].tz = tangent.z;
        verticesOrigin[v].ts = tangentW;
        verticesOrigin[v].tu = texcoord.x;
        verticesOrigin[v].tv = texcoord.y;
    }

    for (int i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace* face = &mesh->mFaces[i];
        assert(face->mNumIndices == 3);
        indicesOrigin[i * 3 + 0] = face->mIndices[0];
        indicesOrigin[i * 3 + 1] = face->mIndices[1];
        indicesOrigin[i * 3 + 2] = face->mIndices[2];
    }

    auto vertexCount = meshopt_generateVertexRemap(remap.data(),
                                                   indicesOrigin.data(),
                                                   indexCount,
                                                   verticesOrigin.data(),
                                                   verticesOrigin.size(),
                                                   sizeof(VertexNonCompressed));

    auto offsetVertices = cluster.vertices.size();
    cluster.vertices.resize(offsetVertices + vertexCount);
    auto vertices = cluster.vertices.data() + offsetVertices;
    std::vector<uint32_t> indices(indexCount);

    meshopt_remapVertexBuffer(
        vertices, verticesOrigin.data(), verticesOrigin.size(), sizeof(VertexNonCompressed), remap.data());
    meshopt_remapIndexBuffer(indices.data(), indicesOrigin.data(), indexCount, remap.data());

    meshopt_optimizeVertexCache(indices.data(), indices.data(), indexCount, vertexCount);
    meshopt_optimizeVertexFetch(
        vertices, indices.data(), indexCount, vertices, vertexCount, sizeof(VertexNonCompressed));

    glm::vec3 center{};
    for (int i = 0; i < vertexCount; ++i) {
        const auto& v = vertices[i];
        center += glm::vec3(v.px, v.py, v.pz);
    }
    center /= float(vertexCount);

    float radius = 0;
    for (int i = 0; i < vertexCount; ++i) {
        const auto& v = vertices[i];

        radius = glm::max(radius, glm::distance(center, glm::vec3(v.px, v.py, v.pz)));
    }

    float lodError         = 0.0f;
    float normalWeights[3] = { 0.5f, 0.5f, 0.5f };

    cluster.meshes.push_back({});
    auto& meshLods        = cluster.meshes.back();
    meshLods.center[0]    = center.x;
    meshLods.center[1]    = center.y;
    meshLods.center[2]    = center.z;
    meshLods.radius       = radius;
    meshLods.bboxMin[0]   = mesh->mAABB.mMin.x;
    meshLods.bboxMin[1]   = mesh->mAABB.mMin.y;
    meshLods.bboxMin[2]   = mesh->mAABB.mMin.z;
    meshLods.bboxMax[0]   = mesh->mAABB.mMax.x;
    meshLods.bboxMax[1]   = mesh->mAABB.mMax.y;
    meshLods.bboxMax[2]   = mesh->mAABB.mMax.z;
    meshLods.vertexOffset = offsetVertices;
    meshLods.vertexCount  = vertexCount;

    auto lodScale = meshopt_simplifyScale(&vertices->px, vertexCount, sizeof(VertexNonCompressed));

    while (meshLods.lodCount < std::size(meshLods.lods)) {
        auto& lod = meshLods.lods[meshLods.lodCount++];

        const auto primitiveOffset = cluster.primitiveIndices.size();

        for (unsigned int i = 0; i < indices.size(); ++i) {
            cluster.primitiveIndices.push_back(indices[i]);
        }

        lod.primitiveOffset = primitiveOffset;
        lod.primitiveCount  = indices.size();
        lod.lodError        = lodError * lodScale;

        if (meshLods.lodCount < std::size(meshLods.lods)) {
            size_t nextIndicesTarget = size_t(double(indices.size()) * 0.75);
            size_t nextIndices       = meshopt_simplifyWithAttributes(indices.data(),
                                                                indices.data(),
                                                                indices.size(),
                                                                &vertices->px,
                                                                vertexCount,
                                                                sizeof(VertexNonCompressed),
                                                                &vertices->nx,
                                                                sizeof(VertexNonCompressed),
                                                                normalWeights,
                                                                3,
                                                                nullptr,
                                                                nextIndicesTarget,
                                                                1e-2f,
                                                                0,
                                                                &lodError);
            assert(nextIndices <= indices.size());

            if (nextIndices == indices.size()) {
                break;
            }

            indices.resize(nextIndices);
            meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertexCount);
        }
    }

    appendMaterial(cache, model, sc, mesh);
}

static void recursiveParsing(Cluster& cluster,
                             Cache& cache,
                             Model& model,
                             const aiScene* sc,
                             const aiNode* nd,
                             gerium_sint32_t parent,
                             gerium_sint32_t level) {
    auto matrix = nd->mTransformation;
    matrix.Transpose();

    glm::mat4 localTransform;
    localTransform[0] = glm::vec4(matrix.a1, matrix.a2, matrix.a3, matrix.a4);
    localTransform[1] = glm::vec4(matrix.b1, matrix.b2, matrix.b3, matrix.b4);
    localTransform[2] = glm::vec4(matrix.c1, matrix.c2, matrix.c3, matrix.c4);
    localTransform[3] = glm::vec4(matrix.d1, matrix.d2, matrix.d3, matrix.d4);

    Model::Node node{};
    node.parent = parent;
    node.level  = level;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(localTransform, node.scale, node.rotation, node.position, skew, perspective);

    auto nodeIndex = (gerium_sint32_t) model.nodes.size();
    model.nodes.push_back(node);
    cache.nodeNames.push_back({});

    if (nd->mName.length) {
        cache.nodeNames.back() = appendString(model, nd->mName.C_Str(), nd->mName.length);
    }

    for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
        if (!cache.meshes.contains(mesh)) {
            cache.meshes.insert(mesh);
            model.meshes.emplace_back((gerium_uint32_t) cluster.meshes.size(), 0, nodeIndex);
            appendMesh(cluster, cache, model, sc, mesh);
        }
    }

    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
        recursiveParsing(cluster, cache, model, sc, nd->mChildren[n], nodeIndex, level + 1);
    }
}

Model loadModel(Cluster& cluster, const entt::hashed_string& name) {
    constexpr auto removePrimitives =
        aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_POLYGON | aiPrimitiveType_NGONEncodingFlag;

    constexpr auto flags = aiProcess_FindInstances | aiProcess_Triangulate | aiProcess_SortByPType |
                           aiProcess_GenBoundingBoxes | aiProcess_MakeLeftHanded | aiProcess_FlipUVs |
                           aiProcess_CalcTangentSpace;

    std::filesystem::path appDir = gerium_file_get_app_dir();

    class IOSystem : public Assimp::IOSystem {
    public:
        IOSystem(const std::filesystem::path& dir) noexcept : _dir(dir) {
        }

        bool Exists(const char* pFile) const override {
            return gerium_file_exists_file(pFile);
        }

        char getOsSeparator() const override {
            return char(std::filesystem::path::preferred_separator);
        }

        Assimp::IOStream* Open(const char* pFile, const char* pMode) override {
            if (strcmp(pMode, "rb") != 0) {
                throw std::runtime_error("IOSystem readonly");
            }
            auto path = (_dir / "models" / pFile).string();

            class IOStream : public Assimp::IOStream {
            public:
                IOStream(gerium_utf8_t path) {
                    check(gerium_file_open(path, true, &_file));
                    _data = (const gerium_uint8_t*) gerium_file_map(_file);
                    _pos  = 0;
                }

                ~IOStream() {
                    gerium_file_destroy(_file);
                }

                size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override {
                    auto count = std::min(pSize * pCount, FileSize() - Tell());
                    if (count) {
                        memcpy(pvBuffer, _data + _pos, count);
                        _pos += count;
                    }
                    return count / (pSize * pCount);
                }

                size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override {
                    throw std::runtime_error("Not implemented");
                }

                aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override {
                    auto offset = (gerium_sint64_t) pOffset;
                    switch (pOrigin) {
                        case aiOrigin_SET:
                            _pos = offset;
                            break;
                        case aiOrigin_CUR:
                            _pos += offset;
                            break;
                        case aiOrigin_END:
                            _pos = FileSize() - offset;
                            break;
                    }
                    return aiReturn_SUCCESS;
                }

                size_t Tell() const override {
                    return _pos;
                }

                size_t FileSize() const override {
                    return gerium_file_get_size(_file);
                }

                void Flush() override {
                }

                gerium_file_t _file;
                const gerium_uint8_t* _data;
                gerium_sint64_t _pos;
            };

            return new IOStream(path.c_str());
        }

        void Close(Assimp::IOStream* pFile) override {
            delete pFile;
        }

        const std::filesystem::path& _dir;
    };

    Assimp::Importer importer;

    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, removePrimitives);
    importer.SetIOHandler(new IOSystem(appDir));

    auto path = (appDir / "models" / (std::string(name.data(), name.size()) + ".gltf")).string();

    gerium_file_t file;
    check(gerium_file_open(path.c_str(), true, &file));
    deferred(gerium_file_destroy(file));

    auto data    = (const void*) gerium_file_map(file);
    auto dataLen = (size_t) gerium_file_get_size(file);

    auto scene = importer.ReadFileFromMemory(data, dataLen, flags);

    Model model{};

    Cache cache{};
    recursiveParsing(cluster, cache, model, scene, scene->mRootNode, -1, 0);

    const auto [nameIndex, nameLen] = appendString(model, name.data(), name.size());

    model.name = entt::hashed_string{ model.strPool.data() + nameIndex, nameLen };

    for (size_t i = 0; i < model.nodes.size(); ++i) {
        auto& node = model.nodes[i];

        const auto [nameIndex, nameLen] = cache.nodeNames[i];

        node.name = entt::hashed_string{ model.strPool.data() + nameIndex, nameLen };
    }

    return model;
}
