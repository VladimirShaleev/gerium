#include "Model.hpp"
#include "glTF.hpp"

using namespace std::string_literals;

PBRMaterial::PBRMaterial(gerium_renderer_t renderer, ResourceManager& resourceManger) :
    _renderer(renderer),
    _resourceManger(&resourceManger) {
}

PBRMaterial::PBRMaterial(const PBRMaterial& pbrMaterial) noexcept {
    copy(pbrMaterial);
}

PBRMaterial& PBRMaterial::operator=(const PBRMaterial& pbrMaterial) noexcept {
    if (this != &pbrMaterial) {
        copy(pbrMaterial);
    }
    return *this;
}

void PBRMaterial::setTechnique(Technique technique) {
    _technique = technique;
}

Technique PBRMaterial::getTechnique() const noexcept {
    return _technique;
}

void PBRMaterial::updateMeshData(const MeshData& meshData) {
    if (!_data) {
        _data = _resourceManger->createBuffer(
            GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "", "mesh_data", nullptr, sizeof(MeshData));
    }
    if (!_descriptorSet) {
        _descriptorSet = _resourceManger->createDescriptorSet();
        gerium_renderer_bind_buffer(_renderer, _descriptorSet, 0, _data);
        gerium_renderer_bind_texture(_renderer, _descriptorSet, 1, _diffuse);
        gerium_renderer_bind_texture(_renderer, _descriptorSet, 2, _normal);
        gerium_renderer_bind_texture(_renderer, _descriptorSet, 3, _roughness);
    }
    auto data = (MeshData*) gerium_renderer_map_buffer(_renderer, _data, 0, 0);
    *data     = meshData;
    gerium_renderer_unmap_buffer(_renderer, _data);
}

DescriptorSet PBRMaterial::getDecriptorSet() const noexcept {
    return _descriptorSet;
}

void PBRMaterial::setDiffuse(Texture handle) noexcept {
    _diffuse = handle;
}

void PBRMaterial::setRoughness(Texture handle) noexcept {
    _roughness = handle;
}

void PBRMaterial::setNormal(Texture handle) noexcept {
    _normal = handle;
}

void PBRMaterial::setOcclusion(Texture handle) noexcept {
    _occlusion = handle;
}

Texture PBRMaterial::getDiffuse() const noexcept {
    return _diffuse;
}

Texture PBRMaterial::getRoughness() const noexcept {
    return _roughness;
}

Texture PBRMaterial::getNormal() const noexcept {
    return _normal;
}

Texture PBRMaterial::getOcclusion() const noexcept {
    return _occlusion;
}

void PBRMaterial::setFactor(const glm::vec4& baseColorFactor,
                            const glm::vec4& metallicRoughnessOcclusionFactor) noexcept {
    _baseColorFactor                  = baseColorFactor;
    _metallicRoughnessOcclusionFactor = metallicRoughnessOcclusionFactor;
}

const glm::vec4& PBRMaterial::getBaseColorFactor() const noexcept {
    return _baseColorFactor;
}

const glm::vec4& PBRMaterial::getMetallicRoughnessOcclusionFactor() const noexcept {
    return _metallicRoughnessOcclusionFactor;
}

void PBRMaterial::setAlpha(gerium_float32_t alphaCutoff, DrawFlags flags) noexcept {
    _alphaCutoff = alphaCutoff;
    _flags       = flags;
}

gerium_float32_t PBRMaterial::getAlphaCutoff() const noexcept {
    return _alphaCutoff;
}

DrawFlags PBRMaterial::getFlags() const noexcept {
    return _flags;
}

void PBRMaterial::copy(const PBRMaterial& pbrMaterial) noexcept {
    _renderer                         = pbrMaterial._renderer;
    _resourceManger                   = pbrMaterial._resourceManger;
    _technique                        = pbrMaterial._technique;
    _diffuse                          = pbrMaterial._diffuse;
    _roughness                        = pbrMaterial._roughness;
    _normal                           = pbrMaterial._normal;
    _occlusion                        = pbrMaterial._occlusion;
    _baseColorFactor                  = pbrMaterial._baseColorFactor;
    _metallicRoughnessOcclusionFactor = pbrMaterial._metallicRoughnessOcclusionFactor;
    _alphaCutoff                      = pbrMaterial._alphaCutoff;
    _flags                            = pbrMaterial._flags;
}

Mesh::Mesh(gerium_renderer_t renderer, ResourceManager& resourceManger) :
    _renderer(renderer),
    _resourceManger(&resourceManger),
    _material(renderer, resourceManger) {
}

Mesh::Mesh(const Mesh& mesh) noexcept : _material(mesh._renderer, *mesh._resourceManger) {
    copy(mesh);
}

Mesh& Mesh::operator=(const Mesh& mesh) noexcept {
    if (this != &mesh) {
        copy(mesh);
    }
    return *this;
}

void Mesh::setNodeIndex(gerium_uint32_t index) noexcept {
    _nodeIndex = index;
}

gerium_uint32_t Mesh::getNodeIndex() const noexcept {
    return _nodeIndex;
}

void Mesh::setMaterial(const PBRMaterial& material) {
    _material = material;
}

PBRMaterial& Mesh::getMaterial() noexcept {
    return _material;
}

void Mesh::setIndices(Buffer indices,
                      gerium_index_type_t type,
                      gerium_uint32_t offset,
                      gerium_uint32_t primitives) noexcept {
    _indices        = indices;
    _indexType      = type;
    _indicesOffset  = offset;
    _primitiveCount = primitives;
}

void Mesh::setPositions(Buffer positions, gerium_uint32_t offset) noexcept {
    _positions       = positions;
    _positionsOffset = offset;
}

void Mesh::setTexcoords(Buffer texcoords, gerium_uint32_t offset) noexcept {
    _texcoords       = texcoords;
    _texcoordsOffset = offset;
}

void Mesh::setNormals(Buffer normals, gerium_uint32_t offset) noexcept {
    _normals       = normals;
    _normalsOffset = offset;
}

void Mesh::setTangents(Buffer tangents, gerium_uint32_t offset) noexcept {
    _tangents       = tangents;
    _tangentsOffset = offset;
}

Buffer Mesh::getIndices() const noexcept {
    return _indices;
}

Buffer Mesh::getPositions() const noexcept {
    return _positions;
}

Buffer Mesh::getTexcoords() const noexcept {
    return _texcoords;
}

Buffer Mesh::getNormals() const noexcept {
    return _normals;
}

Buffer Mesh::getTangents() const noexcept {
    return _tangents;
}

gerium_uint32_t Mesh::getIndicesOffset() const noexcept {
    return _indicesOffset;
}

gerium_uint32_t Mesh::getPositionsOffset() const noexcept {
    return _positionsOffset;
}

gerium_uint32_t Mesh::getTexcoordsOffset() const noexcept {
    return _texcoordsOffset;
}

gerium_uint32_t Mesh::getNormalsOffset() const noexcept {
    return _normalsOffset;
}

gerium_uint32_t Mesh::getTangentsOffset() const noexcept {
    return _tangentsOffset;
}

gerium_index_type_t Mesh::getIndexType() const noexcept {
    return _indexType;
}

gerium_uint32_t Mesh::getPrimitiveCount() const noexcept {
    return _primitiveCount;
}

void Mesh::copy(const Mesh& mesh) noexcept {
    _renderer        = mesh._renderer;
    _resourceManger  = mesh._resourceManger;
    _material        = mesh._material;
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

Model::Model(gerium_renderer_t renderer) : _renderer(renderer) {
}

void Model::addMesh(const Mesh& mesh) {
    _meshes.push_back(mesh);
}

std::vector<Mesh>& Model::meshes() noexcept {
    return _meshes;
}

void Model::resizeNodes(gerium_uint32_t numNodes) {
    _nodes.resize(numNodes);
    _localMatrices.resize(numNodes);
    _worldMatrices.resize(numNodes);
    _inverseWorldMatrices.resize(numNodes);
    _updatedNodes.resize(numNodes);

    for (gerium_uint32_t i = 0; i < numNodes; ++i) {
        _nodes[i].parent = -1;
        _nodes[i].level  = 0;
    }
    _maxLevel = 0;
}

void Model::setHierarchy(gerium_sint32_t nodeIndex, gerium_sint32_t parentIndex, gerium_sint32_t level) noexcept {
    _nodes[nodeIndex].parent = parentIndex;
    _nodes[nodeIndex].level  = level;
    _maxLevel                = std::max(_maxLevel, (gerium_uint32_t) level);
    changeNode(nodeIndex);
}

const Model::Node& Model::getNode(gerium_sint32_t nodeIndex) const noexcept {
    return _nodes[nodeIndex];
}

void Model::setMatrix(gerium_uint32_t nodeIndex, const glm::mat4& mat) {
    _localMatrices[nodeIndex] = mat;
    changeNode(nodeIndex);
}

void Model::updateMatrices(const glm::mat4& parentMat, bool parentUpdated) {
    gerium_uint32_t currentLevel = 0;

    while (currentLevel <= _maxLevel) {
        for (gerium_uint32_t i = 0; i < _nodes.size(); ++i) {
            if (_nodes[i].level != currentLevel) {
                continue;
            }

            if (!_updatedNodes[i] && !parentUpdated) {
                continue;
            }

            _updatedNodes[i] = false;

            if (_nodes[i].parent < 0) {
                _worldMatrices[i] = parentMat * _localMatrices[i];
            } else {
                const auto& parentMatrix = _worldMatrices[_nodes[i].parent];
                _worldMatrices[i]        = parentMatrix * _localMatrices[i];
            }
            _inverseWorldMatrices[i] = glm::inverse(_worldMatrices[i]);
        }

        ++currentLevel;
    }
}

void Model::updateMaterials() {
    for (auto& mesh : _meshes) {
        auto& material = mesh.getMaterial();
        MeshData meshData;
        meshData.world                            = _worldMatrices[mesh.getNodeIndex()];
        meshData.inverseWorld                     = _inverseWorldMatrices[mesh.getNodeIndex()];
        meshData.metallicRoughnessOcclusionFactor = material.getMetallicRoughnessOcclusionFactor();
        material.updateMeshData(meshData);
    }
}

const glm::mat4& Model::getLocalMatrix(gerium_uint32_t nodeIndex) const noexcept {
    return _localMatrices[nodeIndex];
}

const glm::mat4& Model::getWorldMatrix(gerium_uint32_t nodeIndex) const noexcept {
    if (_updatedNodes[nodeIndex]) {
        const_cast<Model*>(this)->updateMatrices();
    }
    return _worldMatrices[nodeIndex];
}

const glm::mat4& Model::getInverseWorldMatrix(gerium_uint32_t nodeIndex) const noexcept {
    if (_updatedNodes[nodeIndex]) {
        const_cast<Model*>(this)->updateMatrices();
    }
    return _inverseWorldMatrices[nodeIndex];
}

static void setSampler(gerium_renderer_t renderer, Texture texture, const gltf::Sampler& sampler) {
    if (!texture) {
        return;
    }
    auto minFilter = GERIUM_FILTER_LINEAR;
    auto magFilter = GERIUM_FILTER_LINEAR;
    auto mipFilter = GERIUM_FILTER_LINEAR;
    auto addressU  = GERIUM_ADDRESS_MODE_REPEAT;
    auto addressV  = GERIUM_ADDRESS_MODE_REPEAT;
    auto addressW  = GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE;

    if (sampler.minFilter == gltf::Filter::Nearest) {
        minFilter = GERIUM_FILTER_NEAREST;
    }

    if (sampler.magFilter == gltf::Filter::Nearest) {
        magFilter = GERIUM_FILTER_NEAREST;
    }

    if (sampler.wrapS == gltf::Wrap::MirroredRepeat) {
        addressU = GERIUM_ADDRESS_MODE_MIRRORED_REPEAT;
    } else if (sampler.wrapS == gltf::Wrap::ClampToEdge) {
        addressU = GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE;
    }

    if (sampler.wrapT == gltf::Wrap::MirroredRepeat) {
        addressV = GERIUM_ADDRESS_MODE_MIRRORED_REPEAT;
    } else if (sampler.wrapT == gltf::Wrap::ClampToEdge) {
        addressV = GERIUM_ADDRESS_MODE_CLAMP_TO_EDGE;
    }

    gerium_renderer_texture_sampler(renderer, texture, minFilter, magFilter, mipFilter, addressU, addressV, addressW);
}

static void fillPbrMaterial(gerium_renderer_t renderer,
                            const gltf::Material& material,
                            PBRMaterial& pbrMaterial,
                            const std::vector<gltf::Texture>& gltfTextures,
                            const std::vector<gltf::Sampler>& gltfSamplers,
                            const std::vector<Texture>& textures) {
    auto flags = material.doubleSided ? DrawFlags::DoubleSided : DrawFlags::None;
    if (material.alphaMode == "MASK") {
        flags |= DrawFlags::AlphaMask;
    } else if (material.alphaMode == "BLEND") {
        flags |= DrawFlags::Transparent;
    }

    const auto alphaCutoff = material.alphaCutoff != gltf::INVALID_FLOAT_VALUE ? material.alphaCutoff : 1.0f;
    pbrMaterial.setAlpha(alphaCutoff, flags);

    glm::vec4 baseColorFactor                  = { 1.0f, 1.0f, 1.0f, 1.0f };
    glm::vec4 metallicRoughnessOcclusionFactor = { 1.0f, 1.0f, 1.0f, 1.0f };

    if (material.pbrMetallicRoughness.has) {
        if (material.pbrMetallicRoughness.baseColorFactor.size() != 0) {
            memcpy(glm::value_ptr(baseColorFactor),
                   material.pbrMetallicRoughness.baseColorFactor.data(),
                   sizeof(glm::vec4));
        }

        metallicRoughnessOcclusionFactor.x = material.pbrMetallicRoughness.roughnessFactor != gltf::INVALID_FLOAT_VALUE
                                                 ? material.pbrMetallicRoughness.roughnessFactor
                                                 : 1.0f;
        metallicRoughnessOcclusionFactor.y = material.pbrMetallicRoughness.metallicFactor != gltf::INVALID_FLOAT_VALUE
                                                 ? material.pbrMetallicRoughness.metallicFactor
                                                 : 1.0f;

        if (material.pbrMetallicRoughness.baseColorTexture.has) {
            const auto& links = gltfTextures[material.pbrMetallicRoughness.baseColorTexture.index];
            pbrMaterial.setDiffuse(textures[links.source]);
            setSampler(renderer, textures[links.source], gltfSamplers[links.sampler]);
        }

        if (material.pbrMetallicRoughness.metallicRoughnessTexture.has) {
            const auto links = gltfTextures[material.pbrMetallicRoughness.metallicRoughnessTexture.index];
            pbrMaterial.setRoughness(textures[links.source]);
            setSampler(renderer, textures[links.source], gltfSamplers[links.sampler]);
        }
    }

    if (material.normalTexture.has) {
        const auto links = gltfTextures[material.normalTexture.index];
        pbrMaterial.setNormal(textures[links.source]);
        setSampler(renderer, textures[links.source], gltfSamplers[links.sampler]);
    }

    if (material.occlusionTexture.has) {
        if (material.occlusionTexture.strength != gltf::INVALID_FLOAT_VALUE) {
            metallicRoughnessOcclusionFactor.z = material.occlusionTexture.strength;
        } else {
            metallicRoughnessOcclusionFactor.z = 1.0f;
        }
    }
    pbrMaterial.setFactor(baseColorFactor, metallicRoughnessOcclusionFactor);
}

Model Model::loadGlTF(gerium_renderer_t renderer, ResourceManager& resourceManager, const std::filesystem::path& path) {
    gltf::glTF glTF{};
    gltf::loadGlTF(glTF, path);

    const auto pathStr = path.string();
    Model model(renderer);

    std::vector<gerium_file_t> bufferFiles;
    std::vector<gerium_uint8_t*> bufferDatas;
    std::vector<Buffer> buffers;
    bufferFiles.resize(glTF.buffers.size());
    bufferDatas.resize(glTF.buffers.size());
    buffers.resize(glTF.bufferViews.size());
    auto i = 0;
    for (const auto& buffer : glTF.buffers) {
        const auto fullPath = (path.parent_path() / buffer.uri).string();
        check(gerium_file_open(fullPath.c_str(), true, &bufferFiles[i]));
        bufferDatas[i] = (gerium_uint8_t*) gerium_file_map(bufferFiles[i]);
        ++i;
    }

    i = 0;
    for (const auto& bufferView : glTF.bufferViews) {
        auto offset     = bufferView.byteOffset == gltf::INVALID_INT_VALUE ? 0 : bufferView.byteOffset;
        auto bufferData = bufferDatas[bufferView.buffer] + offset;
        auto name       = "buffer_"s + (bufferView.name.length() ? bufferView.name : std::to_string(i));

        buffers[i++] = resourceManager.createBuffer(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_INDEX_BIT,
                                                    false,
                                                    pathStr,
                                                    name,
                                                    (gerium_cdata_t) bufferData,
                                                    bufferView.byteLength);
    }

    for (auto& bufferFile : bufferFiles) {
        gerium_file_destroy(bufferFile);
    }

    std::vector<Texture> textures;
    for (const auto& image : glTF.images) {
        const auto fullPath = (path.parent_path() / image.uri).string();
        if (gerium_file_exists_file(fullPath.c_str())) {
            textures.push_back(resourceManager.loadTexture(fullPath));
        } else {
            textures.push_back({});
        }
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

    model.resizeNodes((gerium_uint32_t) numNodes);

    for (const auto& node : root.nodes) {
        nodesToVisit.push(node);
    }

    while (!nodesToVisit.empty()) {
        auto nodeIndex = nodesToVisit.front();
        nodesToVisit.pop();

        auto& node = glTF.nodes[nodeIndex];

        if (node.matrix) {
            model.setMatrix(nodeIndex, node.matrix.value());
        } else {
            auto matS = glm::scale(glm::identity<glm::mat4>(), node.scale);
            auto matT = glm::translate(glm::identity<glm::mat4>(), node.translation);
            auto matR = glm::mat4_cast(node.rotation);
            auto mat  = matS * matR * matT;
            model.setMatrix(nodeIndex, mat);
        }

        if (node.children.size()) {
            const auto& nodeHierarchy = model.getNode(nodeIndex);
            for (const auto& childIndex : node.children) {
                auto& childHierarchy = model.getNode(childIndex);
                model.setHierarchy(childIndex, nodeIndex, nodeHierarchy.level + 1);
                nodesToVisit.push(childIndex);
            }
        }

        if (node.mesh == gltf::INVALID_INT_VALUE) {
            continue;
        }

        auto& gltfMesh = glTF.meshes[node.mesh];

        for (const auto& primitive : gltfMesh.primitives) {
            const auto positionAccessorIndex = gltf::attributeAccessorIndex(
                primitive.attributes.data(), (gerium_uint32_t) primitive.attributes.size(), "POSITION");
            const auto tangentAccessorIndex = gltf::attributeAccessorIndex(
                primitive.attributes.data(), (gerium_uint32_t) primitive.attributes.size(), "TANGENT");
            const auto normalAccessorIndex = gltf::attributeAccessorIndex(
                primitive.attributes.data(), (gerium_uint32_t) primitive.attributes.size(), "NORMAL");
            const auto texcoordAccessorIndex = gltf::attributeAccessorIndex(
                primitive.attributes.data(), (gerium_uint32_t) primitive.attributes.size(), "TEXCOORD_0");

            Buffer positions;
            Buffer tangents;
            Buffer normals;
            Buffer texcoords;
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

            auto& material = glTF.materials[primitive.material];
            PBRMaterial pbrMaterial(renderer, resourceManager);
            fillPbrMaterial(renderer, material, pbrMaterial, glTF.textures, glTF.samplers, textures);

            Mesh mesh(renderer, resourceManager);
            mesh.setNodeIndex(nodeIndex);
            mesh.setPositions(positions, positionsOffset);
            mesh.setTangents(tangents, tangentsOffset);
            mesh.setNormals(normals, normalsOffset);
            mesh.setTexcoords(texcoords, texcoordsOffset);
            mesh.setIndices(indices, indexType, indexOffset, primitiveCount);
            mesh.setMaterial(pbrMaterial);

            model.addMesh(mesh);
        }
    }

    model.updateMatrices();
    return model;
}

void Model::changeNode(gerium_sint32_t nodeIndex) noexcept {
    if (nodeIndex == 0) {
        for (auto i = 0; i < _updatedNodes.size(); ++i) {
            _updatedNodes[i] = true;
        }
    } else {
        std::queue<gerium_sint32_t> indices;
        indices.push(nodeIndex);

        while (!indices.empty()) {
            auto parentIndex = indices.front();
            indices.pop();

            _updatedNodes[parentIndex] = true;

            for (gerium_uint32_t i = 0; i < _nodes.size(); ++i) {
                if (_nodes[i].parent == parentIndex) {
                    indices.push(i);
                }
            }
        }
    }
}
