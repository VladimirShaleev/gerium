#ifndef MODEL_HPP
#define MODEL_HPP

#include "Common.hpp"

struct Cluster {
    std::vector<VertexNonCompressed> vertices;
    std::vector<MeshNonCompressed> meshes;
    std::vector<uint32_t> primitiveIndices;
};

struct Model : private NonCopyable {
    enum class MaterialFlags {
        None        = 0,
        AlphaMask   = 1,
        DoubleSided = 2,
        Transparent = 4
    };

    struct Node {
        gerium_sint32_t parent : 24;
        gerium_sint32_t level  : 8;
        entt::hashed_string name;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    struct Mesh {
        gerium_uint32_t meshIndex;
        gerium_uint32_t materialIndex;
        gerium_sint32_t nodeIndex;
    };

    struct Material {
        entt::hashed_string name = {};

        entt::hashed_string baseColorTexture         = {};
        entt::hashed_string metallicRoughnessTexture = {};
        entt::hashed_string normalTexture            = {};
        entt::hashed_string occlusionTexture         = {};
        entt::hashed_string emissiveTexture          = {};

        glm::vec4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec3 emissiveFactor  = {};

        float metallicFactor    = 1.0f;
        float roughnessFactor   = 1.0f;
        float occlusionStrength = 1.0f;
        float alphaCutoff       = 0.5f;

        MaterialFlags flags = {};
    };

    std::vector<char> strPool;
    entt::hashed_string name;
    std::vector<Node> nodes{};
    std::vector<Mesh> meshes{};
    std::vector<Material> materials{};
};

GERIUM_FLAGS(Model::MaterialFlags);

Model loadModel(Cluster& cluster, const entt::hashed_string& name);

#endif
