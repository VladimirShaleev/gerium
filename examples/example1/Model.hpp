#ifndef MODEL_HPP
#define MODEL_HPP

#include "Primitives.hpp"
#include "ResourceManager.hpp"

struct Mesh {
    Buffer indices{};
    Buffer positions{};
    Buffer texcoords{};
    Buffer normals{};
    Buffer tangents{};

    gerium_index_type_t indexType{};
    gerium_uint32_t primitiveCount{};
    gerium_uint32_t indicesOffset{};
    gerium_uint32_t positionsOffset{};
    gerium_uint32_t texcoordsOffset{};
    gerium_uint32_t normalsOffset{};
    gerium_uint32_t tangentsOffset{};
    BoundingBox bbox{};
};

class Model final : NonMovable {
public:
    explicit Model(gerium_uint32_t numNodes);

    void addMesh(const Mesh& mesh);
    const std::vector<Mesh>& meshes() const noexcept;

    gerium_utf8_t nodeName(gerium_sint32_t nodeIndex) const noexcept;
    gerium_uint32_t nodeNameLength(gerium_sint32_t nodeIndex) const noexcept;
    gerium_sint32_t nodeParent(gerium_sint32_t nodeIndex) const noexcept;
    gerium_sint32_t nodeLevel(gerium_sint32_t nodeIndex) const noexcept;

    void setNodeName(gerium_sint32_t nodeIndex, const std::string& name);
    void setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept;
    void setMatrix(gerium_sint32_t nodeIndex, const glm::mat4& mat);

private:
    struct Node {
        gerium_uint32_t name    : 16;
        gerium_uint32_t nameLen : 16;
        gerium_sint32_t parent  : 24;
        gerium_sint32_t level   : 8;
    };

    std::vector<char> _strPool{};
    std::vector<Mesh> _meshes{};
    std::vector<Node> _nodes{};
    std::vector<glm::mat4> _matrices{};
};

#endif
