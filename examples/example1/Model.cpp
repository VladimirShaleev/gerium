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
    _hash      = 0;
}

const Technique& PBRMaterial::getTechnique() const noexcept {
    return _technique;
}

void PBRMaterial::updateMeshData(const MeshData& meshData) {
    _meshData = meshData;
}

void PBRMaterial::setDiffuse(Texture handle) noexcept {
    _diffuse = handle;
    _hash    = 0;
}

void PBRMaterial::setRoughness(Texture handle) noexcept {
    _roughness = handle;
    _hash      = 0;
}

void PBRMaterial::setNormal(Texture handle) noexcept {
    _normal = handle;
    _hash   = 0;
}

void PBRMaterial::setOcclusion(Texture handle) noexcept {
    _occlusion = handle;
    _hash      = 0;
}

const Texture& PBRMaterial::getDiffuse() const noexcept {
    return _diffuse;
}

const Texture& PBRMaterial::getRoughness() const noexcept {
    return _roughness;
}

const Texture& PBRMaterial::getNormal() const noexcept {
    return _normal;
}

const Texture& PBRMaterial::getOcclusion() const noexcept {
    return _occlusion;
}

void PBRMaterial::setFactor(const glm::vec4& baseColorFactor,
                            const glm::vec4& metallicRoughnessOcclusionFactor) noexcept {
    _baseColorFactor                  = baseColorFactor;
    _metallicRoughnessOcclusionFactor = metallicRoughnessOcclusionFactor;
    _hash                             = 0;
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
    _hash        = 0;
}

gerium_float32_t PBRMaterial::getAlphaCutoff() const noexcept {
    return _alphaCutoff;
}

DrawFlags PBRMaterial::getFlags() const noexcept {
    return _flags;
}

const MeshData& PBRMaterial::meshData() const noexcept {
    return _meshData;
}

gerium_uint64_t PBRMaterial::hash(bool bindlessSupported) const noexcept {
    if (!_hash) {
        auto technique = ((gerium_technique_h) _technique).index;

        _hash = wyhash(&technique, sizeof(technique), 0, _wyp);
        if (!bindlessSupported) {
            auto diffuse   = ((gerium_texture_h) _diffuse).index;
            auto roughness = ((gerium_texture_h) _roughness).index;
            auto normal    = ((gerium_texture_h) _normal).index;
            auto occlusion = ((gerium_texture_h) _occlusion).index;

            _hash = wyhash(&diffuse, sizeof(diffuse), _hash, _wyp);
            _hash = wyhash(&roughness, sizeof(roughness), _hash, _wyp);
            _hash = wyhash(&normal, sizeof(normal), _hash, _wyp);
            _hash = wyhash(&occlusion, sizeof(occlusion), _hash, _wyp);
        }
        // _hash = wyhash(&_baseColorFactor.x, sizeof(_baseColorFactor), _hash, _wyp);
        // _hash = wyhash(&_metallicRoughnessOcclusionFactor.x, sizeof(_metallicRoughnessOcclusionFactor), _hash, _wyp);
        // _hash = wyhash(&_alphaCutoff, sizeof(_alphaCutoff), _hash, _wyp);
        // _hash = wyhash(&_flags, sizeof(_flags), _hash, _wyp);
    }
    return _hash;
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
    _hash     = 0;
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
    _hash           = 0;
}

void Mesh::setPositions(Buffer positions, gerium_uint32_t offset, const glm::vec3& min, const glm::vec3& max) noexcept {
    _positions       = positions;
    _positionsOffset = offset;
    _box             = BoundingBox(min, max);
    _hash            = 0;
}

void Mesh::setTexcoords(Buffer texcoords, gerium_uint32_t offset) noexcept {
    _texcoords       = texcoords;
    _texcoordsOffset = offset;
    _hash            = 0;
}

void Mesh::setNormals(Buffer normals, gerium_uint32_t offset) noexcept {
    _normals       = normals;
    _normalsOffset = offset;
    _hash          = 0;
}

void Mesh::setTangents(Buffer tangents, gerium_uint32_t offset) noexcept {
    _tangents       = tangents;
    _tangentsOffset = offset;
    _hash           = 0;
}

const Buffer& Mesh::getIndices() const noexcept {
    return _indices;
}

const Buffer& Mesh::getPositions() const noexcept {
    return _positions;
}

const Buffer& Mesh::getTexcoords() const noexcept {
    return _texcoords;
}

const Buffer& Mesh::getNormals() const noexcept {
    return _normals;
}

const Buffer& Mesh::getTangents() const noexcept {
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

const BoundingBox& Mesh::boundingBox() const noexcept {
    return _box;
}

const BoundingBox& Mesh::worldBoundingBox() const noexcept {
    return _worldBox;
}

void Mesh::updateBox(const glm::mat4& matrix) noexcept {
    _worldBox = BoundingBox(matrix * glm::vec4(_box.min(), 1.0f), matrix * glm::vec4(_box.max(), 1.0f));
}

bool Mesh::isVisible() const noexcept {
    return _visible;
}

void Mesh::visible(bool show) noexcept {
    _visible = show;
}

gerium_uint64_t Mesh::hash(bool bindlessSupported) const noexcept {
    if (!_hash) {
        auto indices   = ((gerium_buffer_h) _indices).index;
        auto positions = ((gerium_buffer_h) _positions).index;
        auto texcoords = ((gerium_buffer_h) _texcoords).index;
        auto normals   = ((gerium_buffer_h) _normals).index;
        auto tangents  = ((gerium_buffer_h) _tangents).index;

        _hash = _material.hash(bindlessSupported);
        _hash = wyhash(&indices, sizeof(indices), _hash, _wyp);
        _hash = wyhash(&positions, sizeof(positions), _hash, _wyp);
        _hash = wyhash(&texcoords, sizeof(texcoords), _hash, _wyp);
        _hash = wyhash(&normals, sizeof(normals), _hash, _wyp);
        _hash = wyhash(&tangents, sizeof(tangents), _hash, _wyp);
        _hash = wyhash(&_indexType, sizeof(_indexType), _hash, _wyp);
        _hash = wyhash(&_indicesOffset, sizeof(_indicesOffset), _hash, _wyp);
        _hash = wyhash(&_positionsOffset, sizeof(_positionsOffset), _hash, _wyp);
        _hash = wyhash(&_texcoordsOffset, sizeof(_texcoordsOffset), _hash, _wyp);
        _hash = wyhash(&_normalsOffset, sizeof(_normalsOffset), _hash, _wyp);
        _hash = wyhash(&_tangentsOffset, sizeof(_tangentsOffset), _hash, _wyp);
        _hash = wyhash(&_primitiveCount, sizeof(_primitiveCount), _hash, _wyp);
    }
    return _hash;
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
    _box             = mesh._box;
    _worldBox        = mesh._worldBox;
    _visible         = mesh._visible;
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
    _prevWorldMatrices.resize(numNodes);
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

bool Model::updateMatrices(const glm::mat4& parentMat, bool parentUpdated) {
    gerium_uint32_t currentLevel = 0;
    bool hasChanges              = false;
    while (currentLevel <= _maxLevel) {
        for (gerium_uint32_t i = 0; i < _nodes.size(); ++i) {
            if (_nodes[i].level != currentLevel) {
                continue;
            }

            if (!_updatedNodes[i] && !parentUpdated) {
                continue;
            }

            _updatedNodes[i] = false;

            _prevWorldMatrices[i] = _worldMatrices[i];
            if (_nodes[i].parent < 0) {
                _worldMatrices[i] = parentMat * _localMatrices[i];
            } else {
                const auto& parentMatrix = _worldMatrices[_nodes[i].parent];
                _worldMatrices[i]        = parentMatrix * _localMatrices[i];
            }

            _inverseWorldMatrices[i] = glm::inverse(_worldMatrices[i]);
            hasChanges               = true;
        }

        ++currentLevel;
    }

    if (hasChanges) {
        for (auto& mesh : meshes()) {
            const auto& matrix = getWorldMatrix(mesh.getNodeIndex());
            mesh.updateBox(matrix);
        }
    }

    return hasChanges;
}

void Model::updateMaterials() {
    for (auto& mesh : _meshes) {
        const auto nodeIndex = mesh.getNodeIndex();
        auto& material       = mesh.getMaterial();
        MeshData meshData;
        meshData.world        = _worldMatrices[nodeIndex];
        meshData.inverseWorld = _inverseWorldMatrices[nodeIndex];
        meshData.prevWorld    = _worldMatrices[nodeIndex]; // _prevWorldMatrices[nodeIndex];
        material.updateMeshData(meshData);
    }
}

void Model::update(Entity& entity, gerium_data_t data) {
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

const glm::mat4& Model::getPrevWorldMatrix(gerium_uint32_t nodeIndex) const noexcept {
    if (_updatedNodes[nodeIndex]) {
        const_cast<Model*>(this)->updateMatrices();
    }
    return _prevWorldMatrices[nodeIndex];
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

    gerium_renderer_texture_sampler(renderer,
                                    texture,
                                    minFilter,
                                    magFilter,
                                    mipFilter,
                                    addressU,
                                    addressV,
                                    addressW,
                                    GERIUM_REDUCTION_MODE_WEIGHTED_AVERAGE);
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
                                                    "",
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
            glm::vec3 min{};
            glm::vec3 max{};
            getMeshVertexBuffer(glTF, buffers, positionAccessorIndex, positions, positionsOffset, &min, &max);
            getMeshVertexBuffer(glTF, buffers, tangentAccessorIndex, tangents, tangentsOffset);
            getMeshVertexBuffer(glTF, buffers, normalAccessorIndex, normals, normalsOffset);
            getMeshVertexBuffer(glTF, buffers, texcoordAccessorIndex, texcoords, texcoordsOffset);

            if (((gerium_buffer_h) positions).index == UndefinedHandle ||
                ((gerium_buffer_h) tangents).index == UndefinedHandle ||
                ((gerium_buffer_h) normals).index == UndefinedHandle ||
                ((gerium_buffer_h) texcoords).index == UndefinedHandle) {
                continue;
            }

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
            pbrMaterial.setTechnique(resourceManager.getTechniqueByName("base"));

            Mesh mesh(renderer, resourceManager);
            mesh.setNodeIndex(nodeIndex);
            mesh.setPositions(positions, positionsOffset, min, max);
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
