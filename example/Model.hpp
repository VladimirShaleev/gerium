#ifndef MODEL_HPP
#define MODEL_HPP

#include "Common.hpp"

static constexpr gerium_uint16_t UndefinedHandle = std::numeric_limits<gerium_uint16_t>::max();

enum class DrawFlags {
    None        = 0,
    AlphaMask   = 1,
    DoubleSided = 2,
    Transparent = 4
};
GERIUM_FLAGS(DrawFlags);

class PBRMaterial final {
public:
private:
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

class Mesh final {
public:
    explicit Mesh(gerium_renderer_t renderer);
    ~Mesh();

    Mesh(const Mesh& mesh) noexcept;
    Mesh(Mesh&& mesh) noexcept;

    Mesh& operator=(const Mesh& mesh) noexcept;
    Mesh& operator=(Mesh&& mesh) noexcept;

    void setNodeIndex(gerium_uint32_t index) noexcept;
    gerium_uint32_t getNodeIndex() const noexcept;

    void setIndices(gerium_buffer_h indices,
                    gerium_index_type_t type,
                    gerium_uint32_t offset,
                    gerium_uint32_t primitives) noexcept;
    void setPositions(gerium_buffer_h positions, gerium_uint32_t offset) noexcept;
    void setTexcoords(gerium_buffer_h texcoords, gerium_uint32_t offset) noexcept;
    void setNormals(gerium_buffer_h normals, gerium_uint32_t offset) noexcept;
    void setTangents(gerium_buffer_h tangents, gerium_uint32_t offset) noexcept;

    gerium_buffer_h getIndices() const noexcept;
    gerium_buffer_h getPositions() const noexcept;
    gerium_buffer_h getTexcoords() const noexcept;
    gerium_buffer_h getNormals() const noexcept;
    gerium_buffer_h getTangents() const noexcept;

private:
    void copy(const Mesh& mesh) noexcept;
    void reference() noexcept;
    void destroy() noexcept;
    void invalidateBuffers() noexcept;
    void setBuffer(gerium_buffer_h& oldBuffer, gerium_buffer_h newBuffer) noexcept;

    gerium_renderer_t _renderer{};

    // PBRMaterial _material{};

    gerium_buffer_h _indices{ UndefinedHandle };
    gerium_buffer_h _positions{ UndefinedHandle };
    gerium_buffer_h _texcoords{ UndefinedHandle };
    gerium_buffer_h _normals{ UndefinedHandle };
    gerium_buffer_h _tangents{ UndefinedHandle };

    gerium_index_type_t _indexType{};
    gerium_uint32_t _indicesOffset{};
    gerium_uint32_t _positionsOffset{};
    gerium_uint32_t _texcoordsOffset{};
    gerium_uint32_t _normalsOffset{};
    gerium_uint32_t _tangentsOffset{};

    gerium_uint32_t _primitiveCount{};
    gerium_uint32_t _nodeIndex{};
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
    void changeNode(gerium_sint32_t nodeIndex) noexcept;

    std::vector<glm::mat4> localMatrices;
    std::vector<glm::mat4> worldMatrices;
    std::vector<NodeHierarchy> nodesHierarchy;

    std::vector<bool> updatedNodes;
    gerium_uint32_t maxLevel;
};

class Model final {
public:
    explicit Model(gerium_renderer_t renderer);

    void addMesh(const Mesh& mesh);
    std::vector<Mesh>& meshes() noexcept;

    void resizeNodes(gerium_uint32_t numNodes);
    void setNodeMatrix(gerium_uint32_t nodeIndex, const glm::mat4& mat);
    void setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept;
    void updateMatrices();

    const NodeHierarchy& getHierarchy(gerium_sint32_t nodeIndex) const noexcept;

    const glm::mat4& getLocalMatrix(gerium_uint32_t nodeIndex) const noexcept;
    const glm::mat4& getWorldMatrix(gerium_uint32_t nodeIndex) const noexcept;

    static Model loadGlTF(gerium_renderer_t renderer, const std::filesystem::path& path);

private:
    gerium_renderer_t _renderer{};
    std::vector<Mesh> _meshes{};
    mutable Hierarchy _hierarchy{};
};

#endif
