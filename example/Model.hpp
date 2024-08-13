#ifndef MODEL_HPP
#define MODEL_HPP

#include <gerium/gerium.h>

#include <filesystem>
#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_MESSAGES
// https://github.com/g-truc/glm/issues/1269
#include <glm/detail/setup.hpp>
#undef GLM_DEPRECATED
#define GLM_DEPRECATED [[deprecated]]
#include <glm/ext.hpp>

enum class DrawFlags {
    None        = 0,
    AlphaMask   = 1,
    DoubleSided = 2,
    Transparent = 4
};
GERIUM_FLAGS(DrawFlags);

struct PBRMaterial {
    // gerium_technique_h technique;

    // gerium_buffer_h data;
    // gerium_descriptor_set_h descriptorSet;

    // gerium_texture_h diffuse;
    // gerium_texture_h roughness;
    // gerium_texture_h normal;
    // gerium_texture_h occlusion;

    glm::vec4 baseColorFactor;
    glm::vec4 metallicRoughnessOcclusionFactor;

    gerium_float32_t alphaCutoff;
    DrawFlags flags;
};

struct Mesh {
    PBRMaterial material;

    gerium_buffer_h indices;
    gerium_buffer_h positions;
    gerium_buffer_h texcoords;
    gerium_buffer_h normals;
    gerium_buffer_h tangents;

    gerium_index_type_t indexType;
    gerium_uint32_t indexOffset;
    gerium_uint32_t positionOffset;
    gerium_uint32_t texcoordsOffset;
    gerium_uint32_t normalOffset;
    gerium_uint32_t tangentsOffset;

    gerium_uint32_t primitiveCount;
    gerium_uint32_t nodeIndex;
};

struct NodeHierarchy {
    gerium_sint32_t parent : 24;
    gerium_sint32_t level  : 8;
};

struct Hierarchy {
    void resize(gerium_uint32_t numNodes);

    void setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept;
    void setLocalMatrix(gerium_sint32_t nodeIndex, const glm::mat4& localMatrix) noexcept;
    void updateMatrices() noexcept;

    std::vector<glm::mat4> localMatrices;
    std::vector<glm::mat4> worldMatrices;
    std::vector<NodeHierarchy> nodesHierarchy;

    std::vector<bool> updatedNodes;
};

struct DataBuffer {
    struct BufferImpl {
        ~BufferImpl() {
            if (renderer) {
                gerium_renderer_destroy_buffer(renderer, buffer);
                renderer = nullptr;
            }
        }

        gerium_renderer_t renderer{};
        gerium_buffer_h buffer{};
    };
    std::shared_ptr<BufferImpl> bufferImpl;

    DataBuffer() : bufferImpl(std::make_shared<BufferImpl>()) {
    }

    void setBuffer(gerium_renderer_t renderer, gerium_buffer_h buffer) {
        bufferImpl->renderer = renderer;
        bufferImpl->buffer = buffer;
    }

    gerium_buffer_h buffer() noexcept {
        return bufferImpl->buffer;
    }
};

struct Model {
    gerium_renderer_t renderer;
    Hierarchy hierarchy;
    std::vector<DataBuffer> buffers;
    std::vector<Mesh> meshes;
};

inline void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS && result != GERIUM_RESULT_SKIP_FRAME) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

std::shared_ptr<Model> loadGlTF(gerium_renderer_t renderer, const std::filesystem::path& path);

#endif
