#include "ResourceManager.hpp"

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
            if (_ticks - it->second.lastUsed > 240000.0) {
                switch (it->second.type) {
                    case Texture:
                        gerium_renderer_destroy_texture(_renderer, { it->second.handle });
                        break;
                    case Technique:
                        gerium_renderer_destroy_technique(_renderer, { it->second.handle });
                        break;
                }
                keys.push_back(it->first);
            }
        }
    }
    for (const auto& key : keys) {
        auto it = _resources.find(key);
        _mapResource.erase(it->second.type + it->second.handle);
        _resources.erase(it);
    }
}

gerium_texture_h ResourceManager::loadTexture(const std::filesystem::path& path) {
    const auto pathStr = path.string();
    const auto key     = calcKey(pathStr);

    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { it->second.handle };
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
    resource.type      = Texture;
    resource.name      = pathStr;
    resource.key       = key;
    resource.handle    = texture.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResource[resource.type + resource.handle] = &resource;

    return texture;
}

gerium_technique_h ResourceManager::loadTechnique(const std::string& name) {
    const auto key = calcKey(name);
    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { it->second.handle };
    }
    return { UndefinedHandle };
}

gerium_technique_h ResourceManager::createTechnique(const std::string& name,
                                                    const std::vector<gerium_pipeline_t> pipelines) {
    const auto key = calcKey(name);

    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { it->second.handle };
    }

    gerium_technique_h technique;
    check(gerium_renderer_create_technique(
        _renderer, _frameGraph, name.c_str(), (gerium_uint32_t) pipelines.size(), pipelines.data(), &technique));

    auto& resource     = _resources[key];
    resource.type      = Technique;
    resource.name      = name;
    resource.key       = key;
    resource.handle    = technique.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResource[resource.type + resource.handle] = &resource;

    return technique;
}

void ResourceManager::referenceTexture(gerium_texture_h handle) {
    referenceResource(Texture + handle.unused);
}

void ResourceManager::referenceTechnique(gerium_technique_h handle) {
    referenceResource(Technique + handle.unused);
}

void ResourceManager::deleteTexture(gerium_texture_h handle) {
    deleteResource(Texture + handle.unused);
}

void ResourceManager::deleteTechnique(gerium_technique_h handle) {
    deleteResource(Technique + handle.unused);
}

void ResourceManager::referenceResource(gerium_uint16_t handle) {
    if (auto it = _mapResource.find(handle); it != _mapResource.end()) {
        ++it->second->reference;
        it->second->lastUsed = _ticks;
    }
}

void ResourceManager::deleteResource(gerium_uint16_t handle) {
    if (auto it = _mapResource.find(handle); it != _mapResource.end()) {
        --it->second->reference;
        it->second->lastUsed = _ticks;
    }
}

gerium_uint64_t ResourceManager::calcKey(const std::string& path) noexcept {
    return wyhash(path.c_str(), path.length(), 0, _wyp);
}
