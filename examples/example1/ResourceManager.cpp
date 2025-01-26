#include "ResourceManager.hpp"
#include "Converters.hpp"

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
