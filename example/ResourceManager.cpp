#include "ResourceManager.hpp"
#include "Converters.hpp"

void ResourceManager::create(AsyncLoader& loader, gerium_frame_graph_t frameGraph) {
    _renderer   = loader.renderer();
    _frameGraph = frameGraph;
    _loader     = &loader;
    _ticks      = 0.0;
}

void ResourceManager::destroy() {
    update(250000.0);
}

void ResourceManager::update(gerium_float32_t elapsed) {
    _ticks += elapsed;

    std::vector<gerium_uint64_t> keys;
    for (auto it = _resources.begin(); it != _resources.end(); ++it) {
        if (it->second.reference == 0) {
            if (it->second.type == DescriptorSetType) {
                gerium_renderer_destroy_descriptor_set(_renderer, { it->second.handle });
                keys.push_back(it->first);
            } else if (it->second.type == BufferType) {
                gerium_renderer_destroy_buffer(_renderer, { it->second.handle });
                keys.push_back(it->first);
            } else if (_ticks - it->second.lastUsed > 240000.0) {
                switch (it->second.type) {
                    case TextureType:
                        gerium_renderer_destroy_texture(_renderer, { it->second.handle });
                        break;
                    case TechniqueType:
                        gerium_renderer_destroy_technique(_renderer, { it->second.handle });
                        break;
                    // case BufferType:
                    //     gerium_renderer_destroy_buffer(_renderer, { it->second.handle });
                    //     break;
                    default:
                        break;
                }
                keys.push_back(it->first);
            }
        }
    }
    for (const auto& key : keys) {
        auto it = _resources.find(key);
        _mapResources.erase(it->second.type + it->second.handle);
        _resources.erase(it);
    }
}

void ResourceManager::loadFrameGraph(const std::filesystem::path& path) {
    const auto pathStr = path.string();

    gerium_file_t file;
    check(gerium_file_open(pathStr.c_str(), true, &file));
    deferred(gerium_file_destroy(file));

    auto data = gerium_file_map(file);

    YAML::resetBuffer();
    auto yaml = YAML::Load(std::string((const char*) data, gerium_file_get_size(file)));

    const auto nodes = yaml["frame graph"].as<std::vector<YAML::FrameGraphNode>>();

    for (const auto& node : nodes) {
        check(gerium_frame_graph_add_node(_frameGraph,
                                          node.name.c_str(),
                                          node.inputs.size(),
                                          node.inputs.data(),
                                          node.outputs.size(),
                                          node.outputs.data()));
    }

    check(gerium_frame_graph_compile(_frameGraph));
}

Texture ResourceManager::loadTexture(const std::filesystem::path& path) {
    const auto pathStr = path.string();
    const auto key     = calcKey(pathStr);

    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed      = _ticks;
        gerium_texture_h texture = { it->second.handle };
        return { this, { it->second.handle } };
    }

    gerium_file_t file;
    check(gerium_file_open(pathStr.c_str(), true, &file));
    auto size = gerium_file_get_size(file);
    auto data = gerium_file_map(file);
    auto name = path.filename().string();

    int comp, width, height;
    stbi_info_from_memory((const stbi_uc*) data, (int) size, &width, &height, &comp);

    auto mipLevels = calcMipLevels(width, height);

    gerium_texture_info_t info{};
    info.width   = (gerium_uint16_t) width;
    info.height  = (gerium_uint16_t) height;
    info.depth   = 1;
    info.mipmaps = (gerium_uint16_t) mipLevels;
    info.format  = GERIUM_FORMAT_R8G8B8A8_UNORM;
    info.type    = GERIUM_TEXTURE_TYPE_2D;
    info.name    = name.c_str();

    gerium_texture_h texture;
    check(gerium_renderer_create_texture(_renderer, &info, nullptr, &texture));

    _loader->loadTexture(texture, file, data);

    auto& resource     = _resources[key];
    resource.type      = TextureType;
    resource.name      = pathStr;
    resource.key       = key;
    resource.handle    = texture.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResources[resource.type + resource.handle] = &resource;

    return { this, texture };
}

Technique ResourceManager::loadTechnique(const std::filesystem::path& path) {
    const auto pathStr = path.string();

    gerium_file_t file;
    check(gerium_file_open(pathStr.c_str(), true, &file));
    deferred(gerium_file_destroy(file));

    auto data = gerium_file_map(file);

    YAML::resetBuffer();
    auto yaml = YAML::Load(std::string((const char*) data, gerium_file_get_size(file)));

    const auto name      = yaml["name"].as<std::string>();
    const auto pipelines = yaml["pipelines"].as<std::vector<gerium_pipeline_t>>();

    return createTechnique(name, pipelines);
}

Technique ResourceManager::getTechnique(const std::string& name) {
    auto nameTech  = name + "|tech";
    const auto key = calcKey(nameTech);
    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { this, { it->second.handle } };
    }
    return {};
}

Buffer ResourceManager::getBuffer(const std::string& path, const std::string& name) {
    const auto pathName = path + '|' + name;
    const auto key      = calcKey(pathName);
    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { this, { it->second.handle } };
    }
    return {};
}

Texture ResourceManager::createTexture(const gerium_texture_info_t& info, gerium_cdata_t data) {
    const auto name = std::to_string(_resourceCount++) + '|' + (info.name ? info.name : "tex");
    const auto key  = calcKey(name);

    gerium_texture_h texture;
    check(gerium_renderer_create_texture(_renderer, &info, data, &texture));

    auto& resource     = _resources[key];
    resource.type      = TextureType;
    resource.name      = name;
    resource.key       = key;
    resource.handle    = texture.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResources[resource.type + resource.handle] = &resource;

    return { this, texture };
}

Technique ResourceManager::createTechnique(const std::string& name, const std::vector<gerium_pipeline_t>& pipelines) {
    auto nameTech  = name + "|tech";
    const auto key = calcKey(nameTech);

    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { this, { it->second.handle } };
    }

    gerium_technique_h technique;
    check(gerium_renderer_create_technique(
        _renderer, _frameGraph, name.c_str(), (gerium_uint32_t) pipelines.size(), pipelines.data(), &technique));

    auto& resource     = _resources[key];
    resource.type      = TechniqueType;
    resource.name      = nameTech;
    resource.key       = key;
    resource.handle    = technique.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResources[resource.type + resource.handle] = &resource;

    return { this, technique };
}

Buffer ResourceManager::createBuffer(gerium_buffer_usage_flags_t bufferUsage,
                                     gerium_bool_t dynamic,
                                     const std::string& path,
                                     const std::string& name,
                                     gerium_cdata_t data,
                                     gerium_uint32_t size) {
    const auto pathName = (path == "" ? std::to_string(_resourceCount++) : path) + '|' + name;
    const auto key      = calcKey(pathName);

    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { this, { it->second.handle } };
    }

    gerium_buffer_h buffer;
    check(gerium_renderer_create_buffer(_renderer, bufferUsage, dynamic, name.c_str(), data, size, &buffer));

    auto& resource     = _resources[key];
    resource.type      = BufferType;
    resource.name      = pathName;
    resource.key       = key;
    resource.handle    = buffer.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResources[resource.type + resource.handle] = &resource;

    return { this, buffer };
}

DescriptorSet ResourceManager::createDescriptorSet() {
    const auto name = std::to_string(_resourceCount++) + "|ds";
    const auto key  = calcKey(name);

    gerium_descriptor_set_h descriptorSet;
    check(gerium_renderer_create_descriptor_set(_renderer, false, &descriptorSet));

    auto& resource     = _resources[key];
    resource.type      = DescriptorSetType;
    resource.name      = name;
    resource.key       = key;
    resource.handle    = descriptorSet.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResources[resource.type + resource.handle] = &resource;

    return { this, descriptorSet };
}

void ResourceManager::reference(gerium_texture_h handle) {
    referenceResource(TextureType + handle.unused);
}

void ResourceManager::reference(gerium_technique_h handle) {
    referenceResource(TechniqueType + handle.unused);
}

void ResourceManager::reference(gerium_buffer_h handle) {
    referenceResource(BufferType + handle.unused);
}

void ResourceManager::reference(gerium_descriptor_set_h handle) {
    referenceResource(DescriptorSetType + handle.unused);
}

void ResourceManager::destroy(gerium_texture_h handle) {
    destroyResource(TextureType + handle.unused);
}

void ResourceManager::destroy(gerium_technique_h handle) {
    destroyResource(TechniqueType + handle.unused);
}

void ResourceManager::destroy(gerium_buffer_h handle) {
    destroyResource(BufferType + handle.unused);
}

void ResourceManager::destroy(gerium_descriptor_set_h handle) {
    destroyResource(DescriptorSetType + handle.unused);
}

void ResourceManager::referenceResource(gerium_uint16_t handle) {
    if (auto it = _mapResources.find(handle); it != _mapResources.end()) {
        ++it->second->reference;
        it->second->lastUsed = _ticks;
    }
}

void ResourceManager::destroyResource(gerium_uint16_t handle) {
    if (auto it = _mapResources.find(handle); it != _mapResources.end()) {
        --it->second->reference;
        it->second->lastUsed = _ticks;
    }
}

gerium_uint64_t ResourceManager::calcKey(const std::string& path) noexcept {
    return wyhash(path.c_str(), path.length(), 0, _wyp);
}

gerium_uint32_t ResourceManager::_resourceCount = 0;
