#ifndef MESH_HPP
#define MESH_HPP

#include <gerium/gerium.h>

#include <memory>
#include <filesystem>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_MESSAGES
// https://github.com/g-truc/glm/issues/1269
#include <glm/detail/setup.hpp>
#undef GLM_DEPRECATED
#define GLM_DEPRECATED [[deprecated]]
#include <glm/ext.hpp>

struct SceneData {
    glm::mat4 viewProjection;
    glm::vec4 eye;
};

struct MeshData {
    glm::mat4 world;
    glm::mat4 inverseWorld;
};

struct PBRMaterial {
    gerium_technique_h technique;

    gerium_buffer_h         data;
    gerium_descriptor_set_h descriptorSet;

    gerium_texture_h diffuse;
    gerium_texture_h roughness;
    gerium_texture_h normal;
    gerium_texture_h occlusion;
};

struct Mesh {
    PBRMaterial pbrMaterial;

    gerium_buffer_h indices;
    gerium_buffer_h positions;
    gerium_buffer_h texcoors;
    gerium_buffer_h normals;
    gerium_buffer_h tangets;

    gerium_uint32_t indexOffset;
    gerium_uint32_t positionOffset;
    gerium_uint32_t texcoordOffset;
    gerium_uint32_t normalOffset;
    gerium_uint32_t tangentOffset;

    gerium_index_type_t indexType;
    gerium_uint32_t     primitiveCount;
};


inline void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS && result != GERIUM_RESULT_SKIP_FRAME) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

std::unique_ptr<Mesh> loadMesh(gerium_renderer_t renderer, const std::filesystem::path& path);

#endif
