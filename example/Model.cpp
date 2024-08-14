#include "Model.hpp"
#include "glTF.hpp"

using namespace std::string_literals;

Mesh::Mesh(gerium_renderer_t renderer) : _renderer(renderer) {
}

Mesh::~Mesh() {
    destroy();
}

Mesh::Mesh(const Mesh& mesh) noexcept {
    copy(mesh);
    reference();
}

Mesh::Mesh(Mesh&& mesh) noexcept {
    copy(mesh);
    mesh.invalidateBuffers();
}

Mesh& Mesh::operator=(const Mesh& mesh) noexcept {
    if (this != &mesh) {
        destroy();
        copy(mesh);
        reference();
    }
    return *this;
}

Mesh& Mesh::operator=(Mesh&& mesh) noexcept {
    if (this != &mesh) {
        destroy();
        copy(mesh);
        mesh.invalidateBuffers();
    }
    return *this;
}

void Mesh::setNodeIndex(gerium_uint32_t index) noexcept {
    _nodeIndex = index;
}

gerium_uint32_t Mesh::getNodeIndex() const noexcept {
    return _nodeIndex;
}

void Mesh::setIndices(gerium_buffer_h indices,
                      gerium_index_type_t type,
                      gerium_uint32_t offset,
                      gerium_uint32_t primitives) noexcept {
    setBuffer(_indices, indices);
    _indexType      = type;
    _indicesOffset  = offset;
    _primitiveCount = primitives;
}

void Mesh::setPositions(gerium_buffer_h positions, gerium_uint32_t offset) noexcept {
    setBuffer(_positions, positions);
    _positionsOffset = offset;
}

void Mesh::setTexcoords(gerium_buffer_h texcoords, gerium_uint32_t offset) noexcept {
    setBuffer(_texcoords, texcoords);
    _texcoordsOffset = offset;
}

void Mesh::setNormals(gerium_buffer_h normals, gerium_uint32_t offset) noexcept {
    setBuffer(_normals, normals);
    _normalsOffset = offset;
}

void Mesh::setTangents(gerium_buffer_h tangents, gerium_uint32_t offset) noexcept {
    setBuffer(_tangents, tangents);
    _tangentsOffset = offset;
}

gerium_buffer_h Mesh::getIndices() const noexcept {
    return _indices;
}

gerium_buffer_h Mesh::getPositions() const noexcept {
    return _positions;
}

gerium_buffer_h Mesh::getTexcoords() const noexcept {
    return _texcoords;
}

gerium_buffer_h Mesh::getNormals() const noexcept {
    return _normals;
}

gerium_buffer_h Mesh::getTangents() const noexcept {
    return _tangents;
}

void Mesh::copy(const Mesh& mesh) noexcept {
    _renderer        = mesh._renderer;
    _indices         = mesh._indices;
    _positions       = mesh._positions;
    _texcoords       = mesh._texcoords;
    _normals         = mesh._normals;
    _tangents        = mesh._tangents;
    _indexType       = mesh._indexType;
    _indicesOffset   = mesh._indicesOffset;
    _positionsOffset = mesh._positionsOffset;
    _texcoordsOffset = mesh._texcoordsOffset;
    _normalsOffset   = mesh._normalsOffset;
    _tangentsOffset  = mesh._tangentsOffset;
    _primitiveCount  = mesh._primitiveCount;
    _nodeIndex       = mesh._nodeIndex;
}

void Mesh::reference() noexcept {
    if (_indices.unused != UndefinedHandle) {
        gerium_renderer_reference_buffer(_renderer, _indices);
    }
    if (_positions.unused != UndefinedHandle) {
        gerium_renderer_reference_buffer(_renderer, _positions);
    }
    if (_texcoords.unused != UndefinedHandle) {
        gerium_renderer_reference_buffer(_renderer, _texcoords);
    }
    if (_normals.unused != UndefinedHandle) {
        gerium_renderer_reference_buffer(_renderer, _normals);
    }
    if (_tangents.unused != UndefinedHandle) {
        gerium_renderer_reference_buffer(_renderer, _tangents);
    }
}

void Mesh::destroy() noexcept {
    if (_indices.unused != UndefinedHandle) {
        gerium_renderer_destroy_buffer(_renderer, _indices);
        _indices = { UndefinedHandle };
    }
    if (_positions.unused != UndefinedHandle) {
        gerium_renderer_destroy_buffer(_renderer, _positions);
        _positions = { UndefinedHandle };
    }
    if (_texcoords.unused != UndefinedHandle) {
        gerium_renderer_destroy_buffer(_renderer, _texcoords);
        _texcoords = { UndefinedHandle };
    }
    if (_normals.unused != UndefinedHandle) {
        gerium_renderer_destroy_buffer(_renderer, _normals);
        _normals = { UndefinedHandle };
    }
    if (_tangents.unused != UndefinedHandle) {
        gerium_renderer_destroy_buffer(_renderer, _tangents);
        _tangents = { UndefinedHandle };
    }
}

void Mesh::invalidateBuffers() noexcept {
    _indices   = { UndefinedHandle };
    _positions = { UndefinedHandle };
    _texcoords = { UndefinedHandle };
    _normals   = { UndefinedHandle };
    _tangents  = { UndefinedHandle };
}

void Mesh::setBuffer(gerium_buffer_h& oldBuffer, gerium_buffer_h newBuffer) noexcept {
    if (oldBuffer.unused != newBuffer.unused) {
        if (oldBuffer.unused != UndefinedHandle) {
            gerium_renderer_destroy_buffer(_renderer, oldBuffer);
        }
        oldBuffer = newBuffer;
        if (oldBuffer.unused != UndefinedHandle) {
            gerium_renderer_reference_buffer(_renderer, oldBuffer);
        }
    }
}

void Hierarchy::resize(gerium_uint32_t numNodes) {
    nodesHierarchy.resize(numNodes);
    localMatrices.resize(numNodes);
    worldMatrices.resize(numNodes);

    updatedNodes.resize(numNodes);

    memset(nodesHierarchy.data(), 0, numNodes * sizeof(NodeHierarchy));
    for (uint32_t i = 0; i < numNodes; ++i) {
        nodesHierarchy[i].parent = -1;
    }
    maxLevel = 0;
}

void Hierarchy::setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept {
    nodesHierarchy[nodeIndex].parent = parentIndex;
    nodesHierarchy[nodeIndex].level  = level;
    maxLevel                         = std::max(maxLevel, (gerium_uint32_t) level);
    changeNode(nodeIndex);
}

void Hierarchy::setLocalMatrix(gerium_sint32_t nodeIndex, const glm::mat4& localMatrix) noexcept {
    localMatrices[nodeIndex] = localMatrix;
    changeNode(nodeIndex);
}

void Hierarchy::updateMatrices() noexcept {
    gerium_uint32_t currentLevel = 0;

    while (currentLevel <= maxLevel) {
        for (gerium_uint32_t i = 0; i < nodesHierarchy.size(); ++i) {
            if (nodesHierarchy[i].level != currentLevel) {
                continue;
            }

            if (!updatedNodes[i]) {
                continue;
            }

            updatedNodes[i] = false;

            if (nodesHierarchy[i].parent < 0) {
                worldMatrices[i] = localMatrices[i];
            } else {
                const auto& parentMatrix = worldMatrices[nodesHierarchy[i].parent];
                worldMatrices[i]         = parentMatrix * localMatrices[i];
            }
        }

        ++currentLevel;
    }
}

void Hierarchy::changeNode(gerium_sint32_t nodeIndex) noexcept {
    if (nodeIndex == 0) {
        for (auto i = 0; i < updatedNodes.size(); ++i) {
            updatedNodes[i] = true;
        }
    } else {
        std::queue<gerium_sint32_t> indices;
        indices.push(nodeIndex);

        while (!indices.empty()) {
            auto parentIndex = indices.front();
            indices.pop();

            updatedNodes[parentIndex] = true;

            for (gerium_uint32_t i = 0; i < nodesHierarchy.size(); ++i) {
                if (nodesHierarchy[i].parent != parentIndex) {
                    continue;
                }
                indices.push(i);
            }
        }
    }
}

Model::Model(gerium_renderer_t renderer) : _renderer(renderer) {
}

void Model::addMesh(const Mesh& mesh) {
    _meshes.push_back(mesh);
}

std::vector<Mesh>& Model::meshes() noexcept {
    return _meshes;
}

void Model::resizeNodes(gerium_uint32_t numNodes) {
    _hierarchy.resize(numNodes);
}

void Model::setNodeMatrix(gerium_uint32_t nodeIndex, const glm::mat4& mat) {
    _hierarchy.setLocalMatrix(nodeIndex, mat);
}

void Model::setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept {
    _hierarchy.setHierarchy(nodeIndex, parentIndex, level);
}

void Model::updateMatrices() {
    _hierarchy.updateMatrices();
}

const NodeHierarchy& Model::getHierarchy(gerium_sint32_t nodeIndex) const noexcept {
    return _hierarchy.nodesHierarchy[nodeIndex];
}

const glm::mat4& Model::getLocalMatrix(gerium_uint32_t nodeIndex) const noexcept {
    return _hierarchy.localMatrices[nodeIndex];
}

const glm::mat4& Model::getWorldMatrix(gerium_uint32_t nodeIndex) const noexcept {
    if (_hierarchy.updatedNodes[nodeIndex]) {
        _hierarchy.updateMatrices();
    }
    return _hierarchy.worldMatrices[nodeIndex];
}

/*

static void fillPbrMaterial(Material& material, PBRMaterial& pbrMaterial) {
    if (material.alphaMode == "MASK") {
        pbrMaterial.flags |= DrawFlags::AlphaMask;
    } else if (material.alphaMode == "BLEND") {
        pbrMaterial.flags |= DrawFlags::Transparent;
    }

    pbrMaterial.flags |= material.doubleSided ? DrawFlags::DoubleSided : DrawFlags::None;
    pbrMaterial.alphaCutoff = material.alphaCutoff != INVALID_FLOAT_VALUE ? material.alphaCutoff : 1.f;

    if (material.pbrMetallicRoughness.has) {
        if (material.pbrMetallicRoughness.baseColorFactor.size() != 0) {
            memcpy(glm::value_ptr(pbrMaterial.baseColorFactor),
                   material.pbrMetallicRoughness.baseColorFactor.data(),
                   sizeof(glm::vec4));
        } else {
            pbrMaterial.baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
        }

        pbrMaterial.metallicRoughnessOcclusionFactor.x =
            material.pbrMetallicRoughness.roughnessFactor != INVALID_FLOAT_VALUE
                ? material.pbrMetallicRoughness.roughnessFactor
                : 1.0f;
        pbrMaterial.metallicRoughnessOcclusionFactor.y =
            material.pbrMetallicRoughness.metallicFactor != INVALID_FLOAT_VALUE
                ? material.pbrMetallicRoughness.metallicFactor
                : 1.0f;

        // pbrMaterial.diffuseTextureIndex = getMaterialTexture(&material.pbr_metallic_roughness.base_color_texture);
        // pbrMaterial.roughnessTextureIndex =
        //     getMaterialTexture(&material.pbr_metallic_roughness.metallic_roughness_texture);
    }

    // pbrMaterial.occlusionTextureIndex =
    //     getMaterialTexture((material.occlusion_texture.has) ? material.occlusion_texture.index : -1);
    // pbrMaterial.normalTextureIndex =
    //     getMaterialTexture((material.normal_texture.has) ? material.normal_texture.index : -1);

    // if (material.occlusion_texture.has) {
    //     if (material.occlusion_texture.strength != vision_flow::graphic::gltf::INVALID_FLOAT_VALUE) {
    //         pbrMaterial.metallicRoughnessOcclusionFactor.z = material.occlusion_texture.strength;
    //     } else {
    //         pbrMaterial.metallicRoughnessOcclusionFactor.z = 1.0f;
    //     }
    // }
}

*/

Model Model::loadGlTF(gerium_renderer_t renderer, const std::filesystem::path& path) {
    gltf::glTF glTF{};
    gltf::loadGlTF(glTF, path);

    Model model(renderer);

    std::vector<gerium_file_t> bufferFiles;
    std::vector<gerium_uint8_t*> bufferDatas;
    std::vector<gerium_buffer_h> buffers;
    bufferFiles.resize(glTF.buffers.size());
    bufferDatas.resize(glTF.buffers.size());
    buffers.resize(glTF.bufferViews.size());
    auto i = 0;
    for (const auto& buffer : glTF.buffers) {
        const auto fullPath = path.parent_path() / buffer.uri;
        check(gerium_file_open(fullPath.string().c_str(), true, &bufferFiles[i]));
        bufferDatas[i] = (gerium_uint8_t*) gerium_file_map(bufferFiles[i]);
        ++i;
    }

    i = 0;
    for (const auto& bufferView : glTF.bufferViews) {
        auto offset     = bufferView.byteOffset == gltf::INVALID_INT_VALUE ? 0 : bufferView.byteOffset;
        auto bufferData = bufferDatas[bufferView.buffer] + offset;
        auto name       = "buffer_"s + (bufferView.name.length() ? bufferView.name : std::to_string(i));

        gerium_buffer_h buffer;
        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_INDEX_BIT,
                                            false,
                                            name.c_str(),
                                            (gerium_cdata_t) bufferData,
                                            bufferView.byteLength,
                                            &buffers[i++]));
    }

    for (auto& bufferFile : bufferFiles) {
        gerium_file_destroy(bufferFile);
    }

    auto& root    = glTF.scenes[glTF.scene];
    auto numNodes = root.nodes.size();

    std::queue<gerium_uint32_t> nodesToVisit;
    for (const auto& node : root.nodes) {
        nodesToVisit.push(node);
    }

    while (!nodesToVisit.empty()) {
        auto nodeIndex = nodesToVisit.front();
        nodesToVisit.pop();

        auto& node = glTF.nodes[nodeIndex];
        for (const auto& child : node.children) {
            nodesToVisit.push(child);
        }
        numNodes += node.children.size();
    }

    model.resizeNodes(numNodes);

    for (const auto& node : root.nodes) {
        nodesToVisit.push(node);
    }

    while (!nodesToVisit.empty()) {
        auto nodeIndex = nodesToVisit.front();
        nodesToVisit.pop();

        auto& node = glTF.nodes[nodeIndex];

        if (node.matrix) {
            model.setNodeMatrix(nodeIndex, node.matrix.value());
        } else {
            auto matS = glm::scale(glm::identity<glm::mat4>(), node.scale);
            auto matT = glm::translate(glm::identity<glm::mat4>(), node.translation);
            auto matR = glm::mat4_cast(node.rotation);
            auto mat  = matS * matR * matT;
            model.setNodeMatrix(nodeIndex, mat);
        }

        if (node.children.size()) {
            const auto& nodeHierarchy = model.getHierarchy(nodeIndex);
            for (const auto& childIndex : node.children) {
                auto& childHierarchy = model.getHierarchy(childIndex);
                model.setHierarchy(childIndex, nodeIndex, nodeHierarchy.level + 1);
                nodesToVisit.push(childIndex);
            }
        }

        if (node.mesh == gltf::INVALID_INT_VALUE) {
            continue;
        }

        auto& gltfMesh = glTF.meshes[node.mesh];

        for (const auto& primitive : gltfMesh.primitives) {
            const auto positionAccessorIndex =
                gltf::attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "POSITION");
            const auto tangentAccessorIndex =
                gltf::attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "TANGENT");
            const auto normalAccessorIndex =
                gltf::attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "NORMAL");
            const auto texcoordAccessorIndex =
                gltf::attributeAccessorIndex(primitive.attributes.data(), primitive.attributes.size(), "TEXCOORD_0");

            gerium_buffer_h positions;
            gerium_buffer_h tangents;
            gerium_buffer_h normals;
            gerium_buffer_h texcoords;
            gerium_uint32_t positionsOffset;
            gerium_uint32_t tangentsOffset;
            gerium_uint32_t normalsOffset;
            gerium_uint32_t texcoordsOffset;
            getMeshVertexBuffer(glTF, buffers, positionAccessorIndex, positions, positionsOffset);
            getMeshVertexBuffer(glTF, buffers, tangentAccessorIndex, tangents, tangentsOffset);
            getMeshVertexBuffer(glTF, buffers, normalAccessorIndex, normals, normalsOffset);
            getMeshVertexBuffer(glTF, buffers, texcoordAccessorIndex, texcoords, texcoordsOffset);

            auto& indicesAccessor = glTF.accessors[primitive.indices];
            auto indexType        = indicesAccessor.componentType == gltf::ComponentType::UnsignedShort
                                        ? GERIUM_INDEX_TYPE_UINT16
                                        : GERIUM_INDEX_TYPE_UINT32;

            auto& indicesBufferView = glTF.bufferViews[indicesAccessor.bufferView];
            auto indices            = buffers[indicesAccessor.bufferView];
            auto indexOffset = indicesAccessor.byteOffset == gltf::INVALID_INT_VALUE ? 0 : indicesAccessor.byteOffset;
            auto primitiveCount = indicesAccessor.count;

            Mesh mesh(renderer);
            mesh.setNodeIndex(nodeIndex);
            mesh.setPositions(positions, positionsOffset);
            mesh.setTangents(tangents, tangentsOffset);
            mesh.setNormals(normals, normalsOffset);
            mesh.setTexcoords(texcoords, texcoordsOffset);
            mesh.setIndices(indices, indexType, indexOffset, primitiveCount);

            model.addMesh(mesh);

            // auto& material = gltf.materials[primitive.material];
            // fillPbrMaterial(material, mesh.material);
        }
    }

    for (auto buffer : buffers) {
        gerium_renderer_destroy_buffer(renderer, buffer);
    }

    model.updateMatrices();
    return model;
}
