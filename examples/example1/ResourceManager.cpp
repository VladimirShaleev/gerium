#include "ResourceManager.hpp"
#include "Converters.hpp"
#include "Model.hpp"

void ResourceManager::create(gerium_renderer_t renderer, gerium_frame_graph_t frameGraph) {
    _renderer   = renderer;
    _frameGraph = frameGraph;
    _ticks      = 0.0;
}

void ResourceManager::destroy() {
    _ticks = std::numeric_limits<gerium_uint64_t>::max();
    update(0);
}

void ResourceManager::update(gerium_uint64_t elapsedMs) {
    _ticks += elapsedMs;

    std::vector<gerium_uint64_t> deleteQueue;
    for (auto it = _resources.begin(); it != _resources.end(); ++it) {
        if (it->second.references == 0) {
            if (_ticks - it->second.lastTick >= it->second.retentionMs) {
                switch (it->second.type) {
                    case TextureType:
                        gerium_renderer_destroy_texture(_renderer, { it->second.handle });
                        break;
                    case TechniqueType:
                        gerium_renderer_destroy_technique(_renderer, { it->second.handle });
                        break;
                    case BufferType:
                        gerium_renderer_destroy_buffer(_renderer, { it->second.handle });
                        break;
                    case DescriptorSetType:
                        gerium_renderer_destroy_descriptor_set(_renderer, { it->second.handle });
                }
                deleteQueue.push_back(it->first);
            }
        } else {
            it->second.lastTick = _ticks;
        }
    }

    for (const auto key : deleteQueue) {
        auto it = _resources.find(key);
        if (it->second.path) {
            _pathes.erase(it->second.path);
        }
        if (it->second.name) {
            _names.erase(it->second.name);
        }
        _resources.erase(it);
    }

    for (auto it = _models.begin(); it != _models.end();) {
        auto lastTick    = it->second.lastTick;
        auto retentionMs = it->second.retentionMs;

        if (_ticks - lastTick >= retentionMs) {
            delete it->second.model;
            it = _models.erase(it);
        } else {
            ++it;
        }
    }
}

void ResourceManager::loadFrameGraph(const std::string& filename) {
    auto path = calcFrameGraphPath(filename);

    gerium_file_t file;
    check(gerium_file_open(path.c_str(), true, &file));
    deferred(gerium_file_destroy(file));

    auto data = gerium_file_map(file);

    YAML::resetBuffer();
    auto yaml = YAML::Load(std::string((const char*) data, gerium_file_get_size(file)));

    const auto nodes = yaml["frame graph"].as<std::vector<YAML::FrameGraphNode>>();

    for (const auto& node : nodes) {
        check(gerium_frame_graph_add_node(_frameGraph,
                                          node.name.c_str(),
                                          node.compute,
                                          node.inputs.size(),
                                          node.inputs.data(),
                                          node.outputs.size(),
                                          node.outputs.data()));
    }
}

Texture ResourceManager::loadTexture(const std::string& path, gerium_uint64_t retentionMs) {
    if (auto texture = getTextureByPath(path)) {
        return texture;
    }

    auto fullPath = calcTexturePath(path);

    gerium_texture_h texture;
    check(gerium_renderer_async_load_texture(_renderer, fullPath.c_str(), nullptr, nullptr, &texture));

    auto name = std::filesystem::path(path).filename().string();
    addResource(path, name, texture, retentionMs);
    return { this, texture };
}

Technique ResourceManager::loadTechnique(const std::string& filename, gerium_uint64_t retentionMs) {
    if (auto technique = getTechniqueByPath(filename)) {
        return technique;
    }

    std::filesystem::path appDir = gerium_file_get_app_dir();
    auto path                    = calcTechniquePath(filename);

    gerium_file_t file;
    check(gerium_file_open(path.c_str(), true, &file));
    deferred(gerium_file_destroy(file));

    auto data = gerium_file_map(file);

    YAML::resetBuffer();
    auto yaml = YAML::Load(std::string((const char*) data, gerium_file_get_size(file)));

    const auto name      = yaml["name"].as<std::string>();
    const auto pipelines = yaml["pipelines"].as<std::vector<gerium_pipeline_t>>();

    gerium_technique_h technique;
    check(gerium_renderer_create_technique(
        _renderer, _frameGraph, name.c_str(), (gerium_uint32_t) pipelines.size(), pipelines.data(), &technique));

    addResource(filename, name, technique, retentionMs);
    return { this, technique };
}

const Model* ResourceManager::loadModel(const std::string& filename, gerium_uint64_t retentionMs) {
    if (auto it = _models.find(filename); it != _models.end()) {
        it->second.lastTick = _ticks;
        return it->second.model;
    }

    auto path = calcModelPath(filename);

    std::string err, warn;
    tinygltf::Model gltfModel;

    tinygltf::TinyGLTF gltfLoader{};
    gltfLoader.SetImageLoader(
        [](tinygltf::Image* image,
           const int imageIdx,
           std::string* err,
           std::string* warn,
           int reqWidth,
           int reqHeight,
           const unsigned char* bytes,
           int size,
           void* userData) {
        auto path = (std::filesystem::path("models") / image->uri).string();

        auto manager = (ResourceManager*) userData;
        auto texture = manager->loadTexture(path);

        return true;
    },
        this);

    auto result = gltfLoader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);

    if (!err.empty()) {
        throw std::runtime_error(err);
    }

    if (!warn.empty()) {
        throw std::runtime_error(warn);
    }

    if (!result) {
        throw std::runtime_error("Failed to parse glTF: " + filename);
    }

    std::vector<Buffer> buffers(gltfModel.bufferViews.size());

    for (int i = 0; i < gltfModel.bufferViews.size(); ++i) {
        auto view  = gltfModel.bufferViews[i];
        auto data  = (gerium_cdata_t) &gltfModel.buffers[view.buffer].data[view.byteOffset];
        auto name  = "_gltf_" + filename + '_' + (view.name.empty() ? std::to_string(i) : view.name);
        auto usage = view.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER ? GERIUM_BUFFER_USAGE_INDEX_BIT
                                                                         : GERIUM_BUFFER_USAGE_VERTEX_BIT;

        buffers[i] = createBuffer(usage, false, name.c_str(), data, view.byteLength);
    }

    const auto& root = gltfModel.scenes[gltfModel.defaultScene];
    auto numNodes    = root.nodes.size();

    std::queue<gerium_uint32_t> nodesToVisit;
    for (const auto& node : root.nodes) {
        nodesToVisit.push(node);
    }

    while (!nodesToVisit.empty()) {
        auto nodeIndex = nodesToVisit.front();
        nodesToVisit.pop();

        auto& node = gltfModel.nodes[nodeIndex];
        for (const auto& child : node.children) {
            nodesToVisit.push(child);
        }
        numNodes += node.children.size();
    }

    auto& resource       = _models[filename];
    resource.model       = new Model(numNodes);
    resource.lastTick    = _ticks;
    resource.retentionMs = retentionMs;

    auto model = resource.model;

    for (const auto& node : root.nodes) {
        nodesToVisit.push(node);
    }

    while (!nodesToVisit.empty()) {
        auto nodeIndex = nodesToVisit.front();
        nodesToVisit.pop();

        auto& node = gltfModel.nodes[nodeIndex];

        if (node.matrix.empty()) {
            auto scale = node.scale.empty()
                             ? glm::vec3(1.0f)
                             : glm::vec3(float(node.scale[0]), float(node.scale[1]), float(node.scale[2]));
            auto translate =
                node.translation.empty()
                    ? glm::vec3()
                    : glm::vec3(float(node.translation[0]), float(node.translation[1]), float(node.translation[2]));
            auto rotation = node.rotation.empty() ? glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
                                                  : glm::quat(float(node.rotation[0]),
                                                              float(node.rotation[1]),
                                                              float(node.rotation[2]),
                                                              float(node.rotation[3]));

            auto matS = glm::scale(glm::identity<glm::mat4>(), scale);
            auto matT = glm::translate(glm::identity<glm::mat4>(), translate);
            auto matR = glm::mat4_cast(rotation);
            auto mat  = matS * matR * matT;
            model->setMatrix(nodeIndex, mat);
        } else {
            auto data = node.matrix.data();
            auto mat  = glm::mat4(float(data[0]),
                                 float(data[1]),
                                 float(data[2]),
                                 float(data[3]),
                                 float(data[4]),
                                 float(data[5]),
                                 float(data[6]),
                                 float(data[7]),
                                 float(data[8]),
                                 float(data[9]),
                                 float(data[10]),
                                 float(data[11]),
                                 float(data[12]),
                                 float(data[13]),
                                 float(data[14]),
                                 float(data[15]));

            model->setMatrix(nodeIndex, mat);
        }

        model->setNodeName(nodeIndex, node.name);
        if (node.children.size()) {
            const auto& nodeLevel = model->nodeLevel(nodeIndex);
            for (const auto& childIndex : node.children) {
                model->setHierarchy(childIndex, nodeIndex, nodeLevel + 1);
                nodesToVisit.push(childIndex);
            }
        }

        if (node.mesh < 0) {
            continue;
        }

        const auto mesh = gltfModel.meshes[node.mesh];

        for (const auto& primitive : mesh.primitives) {
            const auto positionAccessorIndex = primitive.attributes.at("POSITION");
            const auto tangentAccessorIndex  = primitive.attributes.at("TANGENT");
            const auto normalAccessorIndex   = primitive.attributes.at("NORMAL");
            const auto texcoordAccessorIndex = primitive.attributes.at("TEXCOORD_0");

            auto& positionAccessor = gltfModel.accessors[positionAccessorIndex];
            auto& tangentAccessor  = gltfModel.accessors[tangentAccessorIndex];
            auto& normalAccessor   = gltfModel.accessors[normalAccessorIndex];
            auto& texcoordAccessor = gltfModel.accessors[texcoordAccessorIndex];
            auto& indicesAccessor  = gltfModel.accessors[primitive.indices];
            auto& positionBuffer   = buffers[positionAccessor.bufferView];
            auto& tangentBuffer    = buffers[tangentAccessor.bufferView];
            auto& normalBuffer     = buffers[normalAccessor.bufferView];
            auto& texcoordBuffer   = buffers[texcoordAccessor.bufferView];
            auto& indicesBuffer    = buffers[indicesAccessor.bufferView];
            auto positionOffset    = positionAccessor.byteOffset;
            auto tangentOffset     = tangentAccessor.byteOffset;
            auto normalOffset      = normalAccessor.byteOffset;
            auto texcoordOffset    = texcoordAccessor.byteOffset;
            auto indexOffset       = indicesAccessor.byteOffset;
            auto primitiveCount    = indicesAccessor.count;
            auto indexType         = indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT
                                         ? GERIUM_INDEX_TYPE_UINT16
                                         : GERIUM_INDEX_TYPE_UINT32;

            auto min = glm::vec3(float(positionAccessor.minValues[0]),
                                 float(positionAccessor.minValues[1]),
                                 float(positionAccessor.minValues[2]));
            auto max = glm::vec3(float(positionAccessor.maxValues[0]),
                                 float(positionAccessor.maxValues[1]),
                                 float(positionAccessor.maxValues[2]));

            Mesh mesh{};
            mesh.indices   = indicesBuffer;
            mesh.positions = positionBuffer;
            mesh.texcoords = texcoordBuffer;
            mesh.normals   = normalBuffer;
            mesh.tangents  = tangentBuffer;

            mesh.indexType       = indexType;
            mesh.primitiveCount  = primitiveCount;
            mesh.indicesOffset   = indexOffset;
            mesh.positionsOffset = positionOffset;
            mesh.texcoordsOffset = texcoordOffset;
            mesh.normalsOffset   = normalOffset;
            mesh.tangentsOffset  = tangentOffset;
            mesh.bbox            = BoundingBox(min, max);

            model->addMesh(mesh);
        }
    }

    return model;
}

Texture ResourceManager::createTexture(const gerium_texture_info_t& info,
                                       gerium_cdata_t data,
                                       gerium_uint64_t retentionMs) {
    std::string name = info.name ? info.name : "";
    if (auto texture = getTextureByName(name)) {
        return texture;
    }

    gerium_texture_h texture;
    check(gerium_renderer_create_texture(_renderer, &info, data, &texture));

    addResource("", name, texture, retentionMs);
    return { this, texture };
}

Texture ResourceManager::createTextureView(const std::string& name,
                                           const Texture& texture,
                                           gerium_texture_type_t type,
                                           gerium_uint16_t mipBaseLevel,
                                           gerium_uint16_t mipLevelCount,
                                           gerium_uint16_t layerBase,
                                           gerium_uint16_t layerCount,
                                           gerium_uint64_t retentionMs) {
    gerium_texture_h textureView;
    gerium_renderer_create_texture_view(
        _renderer, texture, type, mipBaseLevel, mipLevelCount, layerBase, layerCount, name.c_str(), &textureView);

    addResource("", name, textureView, retentionMs);
    return { this, textureView };
}

Technique ResourceManager::createTechnique(const std::string& name,
                                           const std::vector<gerium_pipeline_t>& pipelines,
                                           gerium_uint64_t retentionMs) {
    if (auto technique = getTechniqueByName(name)) {
        return technique;
    }

    gerium_technique_h technique;
    check(gerium_renderer_create_technique(
        _renderer, _frameGraph, name.c_str(), (gerium_uint32_t) pipelines.size(), pipelines.data(), &technique));

    addResource("", name, technique, retentionMs);
    return { this, technique };
}

Buffer ResourceManager::createBuffer(gerium_buffer_usage_flags_t bufferUsage,
                                     gerium_bool_t dynamic,
                                     const std::string& name,
                                     gerium_cdata_t data,
                                     gerium_uint32_t size,
                                     gerium_uint64_t retentionMs) {
    if (auto buffer = getBufferByName(name)) {
        return buffer;
    }

    gerium_buffer_h buffer;
    check(gerium_renderer_create_buffer(_renderer, bufferUsage, dynamic, name.c_str(), data, size, &buffer));

    addResource("", name, buffer, dynamic ? 0 : retentionMs);
    return { this, buffer };
}

DescriptorSet ResourceManager::createDescriptorSet(const std::string& name, bool global, gerium_uint64_t retentionMs) {
    if (auto descriptorSet = getDescriptorSetByName(name)) {
        return descriptorSet;
    }

    gerium_descriptor_set_h descriptorSet;
    check(gerium_renderer_create_descriptor_set(_renderer, global, &descriptorSet));

    addResource("", name, descriptorSet, retentionMs);
    return { this, descriptorSet };
}

Texture ResourceManager::getTextureByPath(const std::string& path) {
    return getResourceByPath<gerium_texture_h>(path);
}

Texture ResourceManager::getTextureByName(const std::string& name) {
    return getResourceByName<gerium_texture_h>(name);
}

Technique ResourceManager::getTechniqueByPath(const std::string& filename) {
    return getResourceByPath<gerium_technique_h>(filename);
}

Technique ResourceManager::getTechniqueByName(const std::string& name) {
    return getResourceByName<gerium_technique_h>(name);
}

Buffer ResourceManager::getBufferByPath(const std::string& path) {
    return getResourceByPath<gerium_buffer_h>(path);
}

Buffer ResourceManager::getBufferByName(const std::string& name) {
    return getResourceByName<gerium_buffer_h>(name);
}

DescriptorSet ResourceManager::getDescriptorSetByName(const std::string& name) {
    return getResourceByName<gerium_descriptor_set_h>(name);
}

gerium_uint64_t ResourceManager::calcKey(const std::string& str, Type type) noexcept {
    if (str.length()) {
        return rapidhash_withSeed(str.c_str(), str.length(), (uint64_t) type);
    }
    return 0;
}

std::string ResourceManager::calcFrameGraphPath(const std::string& filename) noexcept {
    std::filesystem::path appDir = gerium_file_get_app_dir();
    return (appDir / "frame-graphs" / (filename + ".yaml")).string();
}

std::string ResourceManager::calcTexturePath(const std::string& filename) noexcept {
    std::filesystem::path appDir = gerium_file_get_app_dir();
    return (appDir / filename).string();
}

std::string ResourceManager::calcTechniquePath(const std::string& filename) noexcept {
    std::filesystem::path appDir = gerium_file_get_app_dir();
    return (appDir / "techniques" / (filename + ".yaml")).string();
}

std::string ResourceManager::calcModelPath(const std::string& filename) noexcept {
    std::filesystem::path appDir = gerium_file_get_app_dir();
    return (appDir / "models" / (filename + ".gltf")).string();
}
