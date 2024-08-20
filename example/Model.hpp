#ifndef MODEL_HPP
#define MODEL_HPP

#include "Common.hpp"
#include "ResourceManager.hpp"

enum class DrawFlags {
    None        = 0,
    AlphaMask   = 1,
    DoubleSided = 2,
    Transparent = 4
};
GERIUM_FLAGS(DrawFlags);

struct MeshData {
    glm::mat4 world;
    glm::mat4 inverseWorld;
};

class PBRMaterial final {
public:
    explicit PBRMaterial(gerium_renderer_t renderer, ResourceManager& resourceManger);
    ~PBRMaterial();

    PBRMaterial(const PBRMaterial& pbrMaterial) noexcept;
    PBRMaterial(PBRMaterial&& pbrMaterial) noexcept;

    PBRMaterial& operator=(const PBRMaterial& pbrMaterial) noexcept;
    PBRMaterial& operator=(PBRMaterial&& pbrMaterial) noexcept;

    void setTechnique(gerium_technique_h technique);
    gerium_technique_h getTechnique() const noexcept;

    void updateMeshData(const MeshData& meshData);
    gerium_descriptor_set_h getDecriptorSet() const noexcept;

    void setDiffuse(gerium_texture_h handle) noexcept;
    void setRoughness(gerium_texture_h handle) noexcept;
    void setNormal(gerium_texture_h handle) noexcept;
    void setOcclusion(gerium_texture_h handle) noexcept;

    gerium_texture_h getDiffuse() const noexcept;
    gerium_texture_h getRoughness() const noexcept;
    gerium_texture_h getNormal() const noexcept;
    gerium_texture_h getOcclusion() const noexcept;

    void setFactor(const glm::vec4& baseColorFactor, const glm::vec4& metallicRoughnessOcclusionFactor) noexcept;
    const glm::vec4& getBaseColorFactor() const noexcept;
    const glm::vec4& getMetallicRoughnessOcclusionFactor() const noexcept;

    void setAlpha(gerium_float32_t alphaCutoff, DrawFlags flags) noexcept;
    gerium_float32_t getAlphaCutoff() const noexcept;
    DrawFlags getFlags() const noexcept;

private:
    void copy(const PBRMaterial& pbrMaterial) noexcept;
    void reference() noexcept;
    void destroy() noexcept;
    void invalidate() noexcept;
    void setTexture(gerium_texture_h& oldTexture, gerium_texture_h newTexture) noexcept;

    gerium_renderer_t _renderer{};
    ResourceManager* _resourceManger{};

    gerium_technique_h _technique{ UndefinedHandle };

    gerium_buffer_h _data{ UndefinedHandle };
    gerium_descriptor_set_h _descriptorSet{ UndefinedHandle };

    gerium_texture_h _diffuse{ UndefinedHandle };
    gerium_texture_h _roughness{ UndefinedHandle };
    gerium_texture_h _normal{ UndefinedHandle };
    gerium_texture_h _occlusion{ UndefinedHandle };

    glm::vec4 _baseColorFactor{};
    glm::vec4 _metallicRoughnessOcclusionFactor{};

    gerium_float32_t _alphaCutoff{};
    DrawFlags _flags{ DrawFlags::None };
};

class Mesh final {
public:
    explicit Mesh(gerium_renderer_t renderer, ResourceManager& resourceManger);
    ~Mesh();

    Mesh(const Mesh& mesh) noexcept;
    Mesh(Mesh&& mesh) noexcept;

    Mesh& operator=(const Mesh& mesh) noexcept;
    Mesh& operator=(Mesh&& mesh) noexcept;

    void setNodeIndex(gerium_uint32_t index) noexcept;
    gerium_uint32_t getNodeIndex() const noexcept;

    void setMaterial(const PBRMaterial& material);
    PBRMaterial& getMaterial() noexcept;

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

    gerium_uint32_t getIndicesOffset() const noexcept;
    gerium_uint32_t getPositionsOffset() const noexcept;
    gerium_uint32_t getTexcoordsOffset() const noexcept;
    gerium_uint32_t getNormalsOffset() const noexcept;
    gerium_uint32_t getTangentsOffset() const noexcept;

    gerium_index_type_t getIndexType() const noexcept;
    gerium_uint32_t getPrimitiveCount() const noexcept;

private:
    void copy(const Mesh& mesh) noexcept;
    void reference() noexcept;
    void destroy() noexcept;
    void invalidateBuffers() noexcept;
    void setBuffer(gerium_buffer_h& oldBuffer, gerium_buffer_h newBuffer) noexcept;

    gerium_renderer_t _renderer{};
    ResourceManager* _resourceManger{};

    PBRMaterial _material;

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

class Model final {
public:
    struct Node {
        gerium_sint32_t parent : 24;
        gerium_sint32_t level  : 8;
    };

    explicit Model(gerium_renderer_t renderer);

    void addMesh(const Mesh& mesh);
    std::vector<Mesh>& meshes() noexcept;

    void resizeNodes(gerium_uint32_t numNodes);
    void setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept;

    const Node& getNode(gerium_sint32_t nodeIndex) const noexcept;

    void setMatrix(gerium_uint32_t nodeIndex, const glm::mat4& mat);
    void updateMatrices(const glm::mat4& parentMat = glm::identity<glm::mat4>(), bool parentUpdated = false);
    void updateMaterials();

    const glm::mat4& getLocalMatrix(gerium_uint32_t nodeIndex) const noexcept;
    const glm::mat4& getWorldMatrix(gerium_uint32_t nodeIndex) const noexcept;
    const glm::mat4& getInverseWorldMatrix(gerium_uint32_t nodeIndex) const noexcept;

    static Model loadGlTF(gerium_renderer_t renderer, ResourceManager& resourceManager, const std::filesystem::path& path);

private:
    void changeNode(gerium_sint32_t nodeIndex) noexcept;

    gerium_renderer_t _renderer{};
    std::vector<Mesh> _meshes{};

    std::vector<Node> _nodes;
    std::vector<glm::mat4> _localMatrices;
    std::vector<glm::mat4> _worldMatrices;
    std::vector<glm::mat4> _inverseWorldMatrices;
    std::vector<bool> _updatedNodes;
    gerium_uint32_t _maxLevel;
};

#endif
