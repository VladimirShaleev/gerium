#include <filesystem>
#include <fstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <argparse/argparse.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/ext.hpp>
#include <meshoptimizer.h>

#include "../shaders/common/types.h"

struct Cluster {
    std::vector<VertexNonCompressed> vertices;
    std::vector<MeshletNonCompressed> meshlets;
    std::vector<MeshNonCompressed> meshes;
    std::vector<uint32_t> vertexIndices;
    std::vector<uint8_t> primitiveIndices;
    std::vector<Instance> instances;
};

struct Cache {
    std::unordered_map<const aiMesh*, Instance> instances{};
    std::unordered_map<const aiMesh*, uint32_t> maxMeshlets{};
    uint32_t meshletVisibilityOffset{};
};

struct Configuration {
    size_t maxVertices;
    size_t maxTriangles;
    float coneWeight;
    float normalWeight;
};

static size_t appendMeshlets(Cluster& cluster,
                             const VertexNonCompressed* vertices,
                             size_t verticesOffset,
                             size_t verticesCount,
                             const Configuration& config,
                             const std::vector<uint32_t>& indices) {
    std::vector<meshopt_Meshlet> meshlets(
        meshopt_buildMeshletsBound(indices.size(), config.maxVertices, config.maxTriangles));
    std::vector<unsigned int> meshletVertices(meshlets.size() * config.maxVertices);
    std::vector<unsigned char> meshletTriangles(meshlets.size() * config.maxTriangles * 3);

    meshlets.resize(meshopt_buildMeshlets(meshlets.data(),
                                          meshletVertices.data(),
                                          meshletTriangles.data(),
                                          indices.data(),
                                          indices.size(),
                                          &vertices[0].px,
                                          verticesCount,
                                          sizeof(VertexNonCompressed),
                                          config.maxVertices,
                                          config.maxTriangles,
                                          config.coneWeight));

    for (auto& meshlet : meshlets) {
        meshopt_optimizeMeshlet(&meshletVertices[meshlet.vertex_offset],
                                &meshletTriangles[meshlet.triangle_offset],
                                meshlet.triangle_count,
                                meshlet.vertex_count);

        const auto vertexOffset    = cluster.vertexIndices.size();
        const auto primitiveOffset = cluster.primitiveIndices.size();

        for (unsigned int i = 0; i < meshlet.vertex_count; ++i) {
            cluster.vertexIndices.push_back(verticesOffset + meshletVertices[meshlet.vertex_offset + i]);
        }

        for (unsigned int i = 0; i < meshlet.triangle_count * 3; ++i) {
            cluster.primitiveIndices.push_back(meshletTriangles[meshlet.triangle_offset + i]);
        }

        const auto bounds = meshopt_computeMeshletBounds(&meshletVertices[meshlet.vertex_offset],
                                                         &meshletTriangles[meshlet.triangle_offset],
                                                         meshlet.triangle_count,
                                                         &vertices[0].px,
                                                         verticesCount,
                                                         sizeof(VertexNonCompressed));

        MeshletNonCompressed meshletInfo = {};
        meshletInfo.vertexOffset         = uint32_t(vertexOffset);
        meshletInfo.primitiveOffset      = uint32_t(primitiveOffset);
        meshletInfo.vertexCount          = meshlet.vertex_count;
        meshletInfo.primitiveCount       = meshlet.triangle_count;

        meshletInfo.center[0]   = bounds.center[0];
        meshletInfo.center[1]   = bounds.center[1];
        meshletInfo.center[2]   = bounds.center[2];
        meshletInfo.radius      = bounds.radius;
        meshletInfo.coneAxis[0] = bounds.cone_axis[0];
        meshletInfo.coneAxis[1] = bounds.cone_axis[1];
        meshletInfo.coneAxis[2] = bounds.cone_axis[2];
        meshletInfo.coneCutoff  = bounds.cone_cutoff;

        cluster.meshlets.push_back(meshletInfo);
    }

    return meshlets.size();
}

void appendMesh(Cluster& cluster, Cache& cache, const Configuration& config, const aiScene* sc, const aiMesh* mesh) {
    if (cache.instances.count(mesh)) {
        return;
    }

    Instance instance{};
    uint32_t maxMeshlets = 0;

    auto indexCount = mesh->mNumFaces * 3;
    std::vector<uint32_t> remap(indexCount);
    std::vector<uint32_t> indicesOrigin(indexCount);
    std::vector<VertexNonCompressed> verticesOrigin(mesh->mNumVertices);

    for (int v = 0; v < mesh->mNumVertices; ++v) {
        const auto vertex   = mesh->mVertices[v];
        const auto normal   = mesh->mNormals[v];
        const auto texcoord = mesh->mTextureCoords[0][v];

        verticesOrigin[v].px = vertex.x;
        verticesOrigin[v].py = vertex.y;
        verticesOrigin[v].pz = vertex.z;
        verticesOrigin[v].nx = normal.x;
        verticesOrigin[v].ny = normal.y;
        verticesOrigin[v].nz = normal.z;
        verticesOrigin[v].tu = texcoord.x;
        verticesOrigin[v].tv = 1.0f - texcoord.y;
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
        radius        = glm::max(radius, glm::distance(center, glm::vec3(v.px, v.py, v.pz)));
    }

    float lodError         = 0.0f;
    float normalWeights[3] = { config.normalWeight, config.normalWeight, config.normalWeight };

    instance.mesh = glm::uint(cluster.meshes.size());
    cluster.meshes.push_back({});

    auto material    = sc->mMaterials[mesh->mMaterialIndex];
    auto numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);
    aiString textureName;
    if (numTextures > 0) {
        material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
    }

    auto& meshLods      = cluster.meshes.back();
    meshLods.center[0]  = center.x;
    meshLods.center[1]  = center.y;
    meshLods.center[2]  = center.z;
    meshLods.radius     = radius;
    meshLods.bboxMin[0] = mesh->mAABB.mMin.x;
    meshLods.bboxMin[1] = mesh->mAABB.mMin.y;
    meshLods.bboxMin[2] = mesh->mAABB.mMin.z;
    meshLods.bboxMax[0] = mesh->mAABB.mMax.x;
    meshLods.bboxMax[1] = mesh->mAABB.mMax.y;
    meshLods.bboxMax[2] = mesh->mAABB.mMax.z;

    auto lodScale = meshopt_simplifyScale(&vertices->px, vertexCount, sizeof(VertexNonCompressed));

    while (meshLods.lodCount < std::size(meshLods.lods)) {
        auto& lod = meshLods.lods[meshLods.lodCount++];

        lod.meshletOffset = uint32_t(cluster.meshlets.size());
        lod.meshletCount  = uint32_t(appendMeshlets(cluster, vertices, offsetVertices, vertexCount, config, indices));
        lod.lodError      = lodError * lodScale;
        maxMeshlets       = std::max(maxMeshlets, lod.meshletCount);

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

    cache.instances[mesh]   = instance;
    cache.maxMeshlets[mesh] = maxMeshlets;
}

static void recursiveParsing(Cluster& cluster,
                             Cache& cache,
                             const Configuration& config,
                             const aiScene* sc,
                             const aiNode* nd,
                             const glm::mat4& parentTransform) {
    auto matrix = nd->mTransformation;
    matrix.Transpose();

    glm::mat4 localTransform;
    localTransform[0] = glm::vec4(matrix.a1, matrix.a2, matrix.a3, matrix.a4);
    localTransform[1] = glm::vec4(matrix.b1, matrix.b2, matrix.b3, matrix.b4);
    localTransform[2] = glm::vec4(matrix.c1, matrix.c2, matrix.c3, matrix.c4);
    localTransform[3] = glm::vec4(matrix.d1, matrix.d2, matrix.d3, matrix.d4);

    const auto worldTransform = parentTransform * localTransform;

    for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
        const aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

        appendMesh(cluster, cache, config, sc, mesh);
        Instance newInstance = cache.instances[mesh];
        uint32_t maxMeshlets = cache.maxMeshlets[mesh];

        glm::quat rot{};
        glm::vec3 scale{};
        glm::vec3 translate{};
        glm::vec3 skew{};
        glm::vec4 perspective{};
        glm::decompose(worldTransform, scale, rot, translate, skew, perspective);

        newInstance.world            = worldTransform;
        newInstance.inverseWorld     = glm::inverse(newInstance.world);
        newInstance.scale            = glm::max(glm::max(scale.x, scale.y), scale.z);
        newInstance.visibilityOffset = cache.meshletVisibilityOffset;

        cache.meshletVisibilityOffset += maxMeshlets;

        cluster.instances.push_back(newInstance);
    }

    for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
        recursiveParsing(cluster, cache, config, sc, nd->mChildren[n], worldTransform);
    }
}

static Cluster packCluster(const std::string& path, const Configuration& config) {
    constexpr auto removePrimitives =
        aiPrimitiveType_POINT | aiPrimitiveType_LINE | aiPrimitiveType_POLYGON | aiPrimitiveType_NGONEncodingFlag;

    constexpr auto flags =
        aiProcess_FindInstances | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenBoundingBoxes;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, removePrimitives);

    auto scene = importer.ReadFile(path, flags);

    glm::mat4 transform = glm::identity<glm::mat4>();

    Cluster cluster{};
    Cache cache{};
    recursiveParsing(cluster, cache, config, scene, scene->mRootNode, transform);
    importer.FreeScene();

    return cluster;
}

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("cluster-pack", "1.0.0");

    std::string path;
    std::string out;
    bool compress       = false;
    size_t maxVertices  = 64;
    size_t maxTriangles = 124;
    double coneWeight   = 0.5f;
    double normalWeight = 0.5f;
    program.add_argument("scene").help("path to glTF scene").required().store_into(path);
    program.add_argument("-o", "--out").help("out path").store_into(out);
    program.add_argument("-c", "--compress")
        .help("compact packaging of vertices")
        .implicit_value(true)
        .store_into(compress);
    program.add_argument("--mvertices").help("maximum number of vertices in a meshlet").store_into(maxVertices);
    program.add_argument("--mtriangles").help("maximum number of triangles in a meshlet").store_into(maxTriangles);
    program.add_argument("--cweight")
        .help("cone weight for calc cutoff angle of the first meshlet's normal cone")
        .store_into(coneWeight);
    program.add_argument("--nweight").help("normal weight").store_into(normalWeight);

    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    auto cluster = packCluster(path,
                               Configuration{ .maxVertices  = maxVertices,
                                              .maxTriangles = maxTriangles,
                                              .coneWeight   = float(coneWeight),
                                              .normalWeight = float(normalWeight) });

    auto dir = out.length() ? out : std::filesystem::current_path();
    std::filesystem::path name;
    if (dir.has_filename() && !std::filesystem::is_directory(dir)) {
        name = dir.filename().replace_extension();
        dir.remove_filename();
    } else {
        name = "result";
    }

    std::ofstream stream(dir / name.replace_extension(".cluster"), std::ios::binary);
    if (stream.fail()) {
        return EXIT_FAILURE;
    }

    auto vertexIndicesSize   = uint32_t(cluster.vertexIndices.size() * sizeof(cluster.vertexIndices[0]));
    auto primitivIndicesSize = uint32_t(cluster.primitiveIndices.size() * sizeof(cluster.primitiveIndices[0]));
    auto instancesSize       = uint32_t(cluster.instances.size() * sizeof(cluster.instances[0]));

    if (compress) {
        auto verticesSize = uint32_t(cluster.vertices.size() * sizeof(VertexCompressed));
        auto meshletsSize = uint32_t(cluster.meshlets.size() * sizeof(MeshletCompressed));
        auto meshesSize   = uint32_t(cluster.meshes.size() * sizeof(MeshCompressed));
        stream.write((const char*) &verticesSize, sizeof(verticesSize));
        stream.write((const char*) &meshletsSize, sizeof(meshletsSize));
        stream.write((const char*) &meshesSize, sizeof(meshesSize));
        stream.write((const char*) &vertexIndicesSize, sizeof(vertexIndicesSize));
        stream.write((const char*) &primitivIndicesSize, sizeof(primitivIndicesSize));
        stream.write((const char*) &instancesSize, sizeof(instancesSize));

        for (const auto& v : cluster.vertices) {
            auto n = glm::vec3(v.nx, v.ny, v.nz) * 127.0f + 127.5f;
            VertexCompressed vc{};
            vc.px = meshopt_quantizeHalf(v.px);
            vc.py = meshopt_quantizeHalf(v.py);
            vc.pz = meshopt_quantizeHalf(v.pz);
            vc.nx = uint8_t(n.x);
            vc.ny = uint8_t(n.y);
            vc.nz = uint8_t(n.z);
            vc.tu = meshopt_quantizeHalf(v.tu);
            vc.tv = meshopt_quantizeHalf(v.tv);
            stream.write((const char*) &vc, sizeof(vc));
        }

        for (const auto& m : cluster.meshlets) {
            auto coneAxis8X = (int8_t) meshopt_quantizeSnorm(m.coneAxis[0], 8);
            auto coneAxis8Y = (int8_t) meshopt_quantizeSnorm(m.coneAxis[1], 8);
            auto coneAxis8Z = (int8_t) meshopt_quantizeSnorm(m.coneAxis[2], 8);

            float coneAxis8eX = fabsf(coneAxis8X / 127.f - m.coneAxis[0]);
            float coneAxis8eY = fabsf(coneAxis8Y / 127.f - m.coneAxis[1]);
            float coneAxis8eZ = fabsf(coneAxis8Z / 127.f - m.coneAxis[2]);

            int coneCutoff8 = int(127 * (m.coneCutoff + coneAxis8eX + coneAxis8eY + coneAxis8eZ) + 1);

            MeshletCompressed mc{};
            mc.center[0]       = meshopt_quantizeHalf(m.center[0]);
            mc.center[1]       = meshopt_quantizeHalf(m.center[1]);
            mc.center[2]       = meshopt_quantizeHalf(m.center[2]);
            mc.radius          = meshopt_quantizeHalf(m.radius);
            mc.coneAxis[0]     = coneAxis8X;
            mc.coneAxis[1]     = coneAxis8Y;
            mc.coneAxis[2]     = coneAxis8Z;
            mc.coneCutoff      = (coneCutoff8 > 127) ? 127 : (signed char) (coneCutoff8);
            mc.vertexOffset    = m.vertexOffset;
            mc.primitiveOffset = m.primitiveOffset;
            mc.vertexCount     = uint16_t(m.vertexCount);
            mc.primitiveCount  = uint16_t(m.primitiveCount);
            stream.write((const char*) &mc, sizeof(mc));
        }

        for (const auto& m : cluster.meshes) {
            MeshCompressed mc{};
            mc.center[0]  = meshopt_quantizeHalf(m.center[0]);
            mc.center[1]  = meshopt_quantizeHalf(m.center[1]);
            mc.center[2]  = meshopt_quantizeHalf(m.center[2]);
            mc.radius     = meshopt_quantizeHalf(m.radius);
            mc.bboxMin[0] = meshopt_quantizeHalf(m.bboxMin[0]);
            mc.bboxMin[1] = meshopt_quantizeHalf(m.bboxMin[1]);
            mc.bboxMin[2] = meshopt_quantizeHalf(m.bboxMin[2]);
            mc.bboxMax[0] = meshopt_quantizeHalf(m.bboxMax[0]);
            mc.bboxMax[1] = meshopt_quantizeHalf(m.bboxMax[1]);
            mc.bboxMax[2] = meshopt_quantizeHalf(m.bboxMax[2]);
            mc.lodCount   = uint8_t(m.lodCount);
            for (int i = 0; i < std::size(mc.lods); ++i) {
                mc.lods[i] = m.lods[i];
            }
            stream.write((const char*) &mc, sizeof(mc));
        }
    } else {
        auto verticesSize = uint32_t(cluster.vertices.size() * sizeof(VertexNonCompressed));
        auto meshletsSize = uint32_t(cluster.meshlets.size() * sizeof(MeshletNonCompressed));
        auto meshesSize   = uint32_t(cluster.meshes.size() * sizeof(MeshNonCompressed));
        stream.write((const char*) &verticesSize, sizeof(verticesSize));
        stream.write((const char*) &meshletsSize, sizeof(meshletsSize));
        stream.write((const char*) &meshesSize, sizeof(meshesSize));
        stream.write((const char*) &vertexIndicesSize, sizeof(vertexIndicesSize));
        stream.write((const char*) &primitivIndicesSize, sizeof(primitivIndicesSize));
        stream.write((const char*) &instancesSize, sizeof(instancesSize));

        stream.write((const char*) cluster.vertices.data(), cluster.vertices.size() * sizeof(cluster.vertices[0]));
        stream.write((const char*) cluster.meshlets.data(), cluster.meshlets.size() * sizeof(cluster.meshlets[0]));
        stream.write((const char*) cluster.meshes.data(), cluster.meshes.size() * sizeof(cluster.meshes[0]));
    }

    stream.write((const char*) cluster.vertexIndices.data(),
                 cluster.vertexIndices.size() * sizeof(cluster.vertexIndices[0]));
    stream.write((const char*) cluster.primitiveIndices.data(),
                 cluster.primitiveIndices.size() * sizeof(cluster.primitiveIndices[0]));
    stream.write((const char*) cluster.instances.data(), cluster.instances.size() * sizeof(cluster.instances[0]));

    return EXIT_SUCCESS;
}
