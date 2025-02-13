#ifndef COMPONENTS_RENDERABLE_HPP
#define COMPONENTS_RENDERABLE_HPP

#include "../Common.hpp"

enum class MaterialFlags {
    None        = 0,
    AlphaMask   = 1,
    DoubleSided = 2,
    Transparent = 4
};
GERIUM_FLAGS(MaterialFlags);

struct MaterialData {
    hashed_string_owner name = {};

    hashed_string_owner baseColorTexture         = {};
    hashed_string_owner metallicRoughnessTexture = {};
    hashed_string_owner normalTexture            = {};
    hashed_string_owner occlusionTexture         = {};
    hashed_string_owner emissiveTexture          = {};

    glm::vec4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec3 emissiveFactor  = {};

    float metallicFactor    = 1.0f;
    float roughnessFactor   = 1.0f;
    float occlusionStrength = 1.0f;
    float alphaCutoff       = 0.5f;

    MaterialFlags flags = {};
};

struct MeshData {
    hashed_string_owner model;
    gerium_uint32_t mesh;
    MaterialData material;
};

struct Renderable {
    std::vector<MeshData> meshes;
};

#endif
