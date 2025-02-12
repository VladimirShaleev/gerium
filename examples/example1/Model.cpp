#include "Model.hpp"

struct Cache {
    struct PBRNames {
        gerium_uint32_t index{};
        std::tuple<size_t, size_t> name{};
        std::optional<std::tuple<size_t, size_t>> baseColorTexture{};
        std::optional<std::tuple<size_t, size_t>> metallicRoughnessTexture{};
        std::optional<std::tuple<size_t, size_t>> normalTexture{};
        std::optional<std::tuple<size_t, size_t>> occlusionTexture{};
        std::optional<std::tuple<size_t, size_t>> emissiveTexture{};
    };

    std::set<const aiMesh*> meshes;

    std::vector<std::tuple<size_t, size_t>> nodeNames;
    std::map<uint32_t, PBRNames> materialNames;
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
        const auto& name     = material->GetName();

        auto materialName = entt::hashed_string{ name.C_Str(), name.length };

        if (auto it = cache.materialNames.find(materialName); it != cache.materialNames.end()) {
            model.meshes.back().materialIndex = it->second.index;
            return;
        }

        const auto basePath = std::filesystem::path("models") / "textures";

        auto getTexture = [&](aiTextureType type) -> std::optional<std::tuple<size_t, size_t>> {
            if (material->GetTextureCount(type)) {
                aiString name;
                material->Get(AI_MATKEY_TEXTURE(type, 0), name);
                auto nameStr = (basePath / name.C_Str()).string();
                return appendString(model, nameStr.c_str(), nameStr.length());
            }
            return std::nullopt;
        };

        auto& materialNames = cache.materialNames[materialName];

        materialNames.index                    = (gerium_uint32_t) model.materials.size();
        materialNames.name                     = appendString(model, name.C_Str(), name.length);
        materialNames.baseColorTexture         = getTexture(aiTextureType_BASE_COLOR);
        materialNames.metallicRoughnessTexture = getTexture(aiTextureType_METALNESS);
        materialNames.normalTexture            = getTexture(aiTextureType_NORMALS);
        materialNames.occlusionTexture         = getTexture(aiTextureType_LIGHTMAP);
        materialNames.emissiveTexture          = getTexture(aiTextureType_EMISSIVE);

        model.materials.push_back({});
        auto& pbr = model.materials.back();

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
            pbr.flags |= Model::MaterialFlags::AlphaMask;
        } else if (alphaModeStr == "BLEND") {
            pbr.flags |= Model::MaterialFlags::Transparent;
        }

        bool doubleSided;
        material->Get(AI_MATKEY_TWOSIDED, doubleSided);
        if (doubleSided) {
            pbr.flags |= Model::MaterialFlags::DoubleSided;
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

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, removePrimitives);

    std::filesystem::path appDir = gerium_file_get_app_dir();

    auto path = (appDir / "models" / (std::string(name.data(), name.size()) + ".gltf")).string();

    auto scene = importer.ReadFile(path, flags);

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

    for (size_t i = 0; i < model.materials.size(); ++i) {
        auto& material = model.materials[i];

        const auto& names =
            std::find_if(cache.materialNames.cbegin(), cache.materialNames.cend(), [i](const auto& item) {
            return item.second.index == i;
        })->second;

        material.name = entt::hashed_string{ model.strPool.data() + std::get<0>(names.name), std::get<1>(names.name) };

        if (names.baseColorTexture) {
            const auto [nameIndex, nameLen] = names.baseColorTexture.value();

            material.baseColorTexture = entt::hashed_string{ model.strPool.data() + nameIndex, nameLen };
        }
        if (names.metallicRoughnessTexture) {
            const auto [nameIndex, nameLen] = names.metallicRoughnessTexture.value();

            material.metallicRoughnessTexture = entt::hashed_string{ model.strPool.data() + nameIndex, nameLen };
        }
        if (names.normalTexture) {
            const auto [nameIndex, nameLen] = names.normalTexture.value();

            material.normalTexture = entt::hashed_string{ model.strPool.data() + nameIndex, nameLen };
        }
        if (names.occlusionTexture) {
            const auto [nameIndex, nameLen] = names.occlusionTexture.value();

            material.occlusionTexture = entt::hashed_string{ model.strPool.data() + nameIndex, nameLen };
        }
        if (names.emissiveTexture) {
            const auto [nameIndex, nameLen] = names.emissiveTexture.value();

            material.emissiveTexture = entt::hashed_string{ model.strPool.data() + nameIndex, nameLen };
        }
    }

    return model;
}
