#ifndef COMPONENTS_RENDERABLE_HPP
#define COMPONENTS_RENDERABLE_HPP

#include "../Common.hpp"

enum class MaterialFlags {
    None        = 0,
    AlphaMask   = 1,
    DoubleSided = 2,
    Transparent = 4
};

struct MaterialData {
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

struct MeshData {
    entt::hashed_string model;
    gerium_uint32_t mesh;
    MaterialData material;
};

struct Renderable {
    std::vector<MeshData> meshes;
};

#endif
