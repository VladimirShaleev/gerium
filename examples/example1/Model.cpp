#include "Model.hpp"

struct Cache {
    std::set<const aiMesh*> meshes;
};

static void appendMesh(Cluster& cluster, Cache& cache, const aiScene* sc, const aiMesh* mesh) {
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

    if (nd->mName.length) {
        node.nameIndex = (gerium_sint32_t) model.strPool.size();
        node.nameLen   = nd->mName.length;
        model.strPool.resize(node.nameIndex + node.nameLen + 1);
        memcpy(model.strPool.data() + node.nameIndex, nd->mName.C_Str(), node.nameLen);
    }

    auto nodeIndex = (gerium_sint32_t) model.nodes.size();
    model.nodes.push_back(node);

    for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
        if (!cache.meshes.contains(mesh)) {
            cache.meshes.insert(mesh);
            model.meshes.emplace_back((gerium_uint32_t) cluster.meshes.size(), nodeIndex);
            appendMesh(cluster, cache, sc, mesh);
        }
    }

    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
        recursiveParsing(cluster, cache, model, sc, nd->mChildren[n], nodeIndex, level + 1);
    }
}

Model loadModel(Cluster& cluster, const std::string& filename) {
    constexpr auto removePrimitives =
        aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_POLYGON | aiPrimitiveType_NGONEncodingFlag;

    constexpr auto flags = aiProcess_FindInstances | aiProcess_Triangulate | aiProcess_SortByPType |
                           aiProcess_GenBoundingBoxes | aiProcess_MakeLeftHanded | aiProcess_FlipUVs |
                           aiProcess_CalcTangentSpace;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, removePrimitives);

    std::filesystem::path appDir = gerium_file_get_app_dir();

    auto path = (appDir / "models" / (filename + ".gltf")).string();

    auto scene = importer.ReadFile(path, flags);

    Model model{};

    Cache cache{};
    recursiveParsing(cluster, cache, model, scene, scene->mRootNode, -1, 0);

    auto strPoolOffset = model.strPool.size();
    model.strPool.resize(strPoolOffset + filename.length() + 1);
    memcpy(model.strPool.data() + strPoolOffset, filename.c_str(), filename.length());
    model.name = entt::hashed_string{ model.strPool.data() + strPoolOffset, filename.length() };

    for (auto& node : model.nodes) {
        node.name = entt::hashed_string{ model.strPool.data() + node.nameIndex, (size_t) node.nameLen };
    }

    return model;
}
