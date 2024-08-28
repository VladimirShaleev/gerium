#ifndef MODEL_HPP
#define MODEL_HPP

#include "Common.hpp"
#include "EntityComponentSystem.hpp"
#include "Primitives.hpp"
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
    glm::vec4 metallicRoughnessOcclusionFactor;
};

class PBRMaterial final {
public:
    PBRMaterial(gerium_renderer_t renderer, ResourceManager& resourceManger);

    PBRMaterial(PBRMaterial&& pbrMaterial) noexcept            = default;
    PBRMaterial& operator=(PBRMaterial&& pbrMaterial) noexcept = default;

    PBRMaterial(const PBRMaterial& pbrMaterial) noexcept;
    PBRMaterial& operator=(const PBRMaterial& pbrMaterial) noexcept;

    void setTechnique(Technique technique);
    const Technique& getTechnique() const noexcept;

    void updateMeshData(const MeshData& meshData);
    DescriptorSet getDecriptorSet() const noexcept;

    void setDiffuse(Texture handle) noexcept;
    void setRoughness(Texture handle) noexcept;
    void setNormal(Texture handle) noexcept;
    void setOcclusion(Texture handle) noexcept;

    const Texture& getDiffuse() const noexcept;
    const Texture& getRoughness() const noexcept;
    const Texture& getNormal() const noexcept;
    const Texture& getOcclusion() const noexcept;

    void setFactor(const glm::vec4& baseColorFactor, const glm::vec4& metallicRoughnessOcclusionFactor) noexcept;
    const glm::vec4& getBaseColorFactor() const noexcept;
    const glm::vec4& getMetallicRoughnessOcclusionFactor() const noexcept;

    void setAlpha(gerium_float32_t alphaCutoff, DrawFlags flags) noexcept;
    gerium_float32_t getAlphaCutoff() const noexcept;
    DrawFlags getFlags() const noexcept;

    const MeshData& meshData() const noexcept;

    gerium_uint64_t hash() const noexcept;

private:
    void copy(const PBRMaterial& pbrMaterial) noexcept;

    gerium_renderer_t _renderer{};
    ResourceManager* _resourceManger{};

    Technique _technique{};

    Buffer _data{};
    DescriptorSet _descriptorSet{};

    Texture _diffuse{};
    Texture _roughness{};
    Texture _normal{};
    Texture _occlusion{};

    glm::vec4 _baseColorFactor{};
    glm::vec4 _metallicRoughnessOcclusionFactor{};

    gerium_float32_t _alphaCutoff{};
    DrawFlags _flags{ DrawFlags::None };

    MeshData _meshData{};

    mutable gerium_uint64_t _hash{};
};

class Mesh final {
public:
    explicit Mesh(gerium_renderer_t renderer, ResourceManager& resourceManger);

    Mesh(Mesh&& mesh) noexcept            = default;
    Mesh& operator=(Mesh&& mesh) noexcept = default;

    Mesh(const Mesh& mesh) noexcept;
    Mesh& operator=(const Mesh& mesh) noexcept;

    void setNodeIndex(gerium_uint32_t index) noexcept;
    gerium_uint32_t getNodeIndex() const noexcept;

    void setMaterial(const PBRMaterial& material);
    PBRMaterial& getMaterial() noexcept;

    void setIndices(Buffer indices,
                    gerium_index_type_t type,
                    gerium_uint32_t offset,
                    gerium_uint32_t primitives) noexcept;
    void setPositions(Buffer positions, gerium_uint32_t offset, const glm::vec3& min, const glm::vec3& max) noexcept;
    void setTexcoords(Buffer texcoords, gerium_uint32_t offset) noexcept;
    void setNormals(Buffer normals, gerium_uint32_t offset) noexcept;
    void setTangents(Buffer tangents, gerium_uint32_t offset) noexcept;

    const Buffer& getIndices() const noexcept;
    const Buffer& getPositions() const noexcept;
    const Buffer& getTexcoords() const noexcept;
    const Buffer& getNormals() const noexcept;
    const Buffer& getTangents() const noexcept;

    gerium_uint32_t getIndicesOffset() const noexcept;
    gerium_uint32_t getPositionsOffset() const noexcept;
    gerium_uint32_t getTexcoordsOffset() const noexcept;
    gerium_uint32_t getNormalsOffset() const noexcept;
    gerium_uint32_t getTangentsOffset() const noexcept;

    gerium_index_type_t getIndexType() const noexcept;
    gerium_uint32_t getPrimitiveCount() const noexcept;

    const BoundingBox& boundingBox() const noexcept;
    const BoundingBox& worldBoundingBox() const noexcept;

    void updateBox(const glm::mat4& matrix) noexcept;

    bool isVisible() const noexcept;
    void visible(bool show) noexcept;

    gerium_uint64_t hash() const noexcept;

private:
    void copy(const Mesh& mesh) noexcept;

    gerium_renderer_t _renderer{};
    ResourceManager* _resourceManger{};

    PBRMaterial _material;

    Buffer _indices{};
    Buffer _positions{};
    Buffer _texcoords{};
    Buffer _normals{};
    Buffer _tangents{};

    gerium_index_type_t _indexType{};
    gerium_uint32_t _indicesOffset{};
    gerium_uint32_t _positionsOffset{};
    gerium_uint32_t _texcoordsOffset{};
    gerium_uint32_t _normalsOffset{};
    gerium_uint32_t _tangentsOffset{};

    gerium_uint32_t _primitiveCount{};
    gerium_uint32_t _nodeIndex{};

    BoundingBox _box;
    BoundingBox _worldBox;
    bool _visible;

    mutable gerium_uint64_t _hash{};
};

class Model final : public Component {
public:
    struct Node {
        gerium_sint32_t parent : 24;
        gerium_sint32_t level  : 8;
    };

    Model() = default;
    explicit Model(gerium_renderer_t renderer);

    void addMesh(const Mesh& mesh);
    std::vector<Mesh>& meshes() noexcept;

    void resizeNodes(gerium_uint32_t numNodes);
    void setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept;

    const Node& getNode(gerium_sint32_t nodeIndex) const noexcept;

    void setMatrix(gerium_uint32_t nodeIndex, const glm::mat4& mat);
    bool updateMatrices(const glm::mat4& parentMat = glm::identity<glm::mat4>(), bool parentUpdated = false);
    void updateMaterials();

    void update(Entity& entity, gerium_data_t data) override;

    const glm::mat4& getLocalMatrix(gerium_uint32_t nodeIndex) const noexcept;
    const glm::mat4& getWorldMatrix(gerium_uint32_t nodeIndex) const noexcept;
    const glm::mat4& getInverseWorldMatrix(gerium_uint32_t nodeIndex) const noexcept;

    static Model loadGlTF(gerium_renderer_t renderer,
                          ResourceManager& resourceManager,
                          const std::filesystem::path& path);

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
