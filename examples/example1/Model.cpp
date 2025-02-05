#include "Model.hpp"

Model::Model(gerium_uint32_t numNodes) {
    _meshes.reserve(numNodes);
    _nodes.resize(numNodes);
    _matrices.resize(numNodes);
    for (gerium_uint32_t i = 0; i < numNodes; ++i) {
        _nodes[i].parent = -1;
        _nodes[i].level  = 0;
        _matrices[i]     = glm::identity<glm::mat4>();
    }
}

void Model::addMesh(const Mesh& mesh) {
    _meshes.push_back(mesh);
}

const std::vector<Mesh>& Model::meshes() const noexcept {
    return _meshes;
}

gerium_utf8_t Model::nodeName(gerium_sint32_t nodeIndex) const noexcept {
    return _nodes[nodeIndex].nameLen != 0 ? &_strPool[_nodes[nodeIndex].name] : nullptr;
}

gerium_uint32_t Model::nodeNameLength(gerium_sint32_t nodeIndex) const noexcept {
    return _nodes[nodeIndex].nameLen;
}

gerium_sint32_t Model::nodeParent(gerium_sint32_t nodeIndex) const noexcept {
    return _nodes[nodeIndex].parent;
}

gerium_sint32_t Model::nodeLevel(gerium_sint32_t nodeIndex) const noexcept {
    return _nodes[nodeIndex].level;
}

void Model::setNodeName(gerium_sint32_t nodeIndex, const std::string& name) {
    if (name.empty()) {
        _nodes[nodeIndex].name    = 0;
        _nodes[nodeIndex].nameLen = 0;
    } else {
        auto offset = gerium_uint32_t(_strPool.size());
        auto length = gerium_uint32_t(name.length());
        _strPool.resize(offset + length + 1);
        memcpy(_strPool.data() + offset, name.data(), length);

        _nodes[nodeIndex].name    = offset;
        _nodes[nodeIndex].nameLen = length;
    }
}

void Model::setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept {
    _nodes[nodeIndex].parent = parentIndex;
    _nodes[nodeIndex].level  = level;
}

void Model::setMatrix(gerium_sint32_t nodeIndex, const glm::mat4& mat) {
    _matrices[nodeIndex] = mat;
}
