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
                    case Buffer:
                        gerium_renderer_destroy_buffer(_renderer, { it->second.handle });
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

gerium_buffer_h ResourceManager::loadBuffer(const std::string& path, const std::string& name) {
    const auto pathName = path + '|' + name;
    const auto key      = calcKey(pathName);
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

gerium_buffer_h ResourceManager::createBuffer(gerium_buffer_usage_flags_t bufferUsage,
                                              gerium_bool_t dynamic,
                                              const std::string& path,
                                              const std::string& name,
                                              gerium_cdata_t data,
                                              gerium_uint32_t size) {
    const auto pathName = (path == "" ? std::to_string(_bufferCount++) : path) + '|' + name;
    const auto key      = calcKey(pathName);

    if (auto it = _resources.find(key); it != _resources.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return { it->second.handle };
    }

    gerium_buffer_h buffer;
    check(gerium_renderer_create_buffer(_renderer, bufferUsage, dynamic, name.c_str(), data, size, &buffer));

    auto& resource     = _resources[key];
    resource.type      = Buffer;
    resource.name      = pathName;
    resource.key       = key;
    resource.handle    = buffer.unused;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _mapResource[resource.type + resource.handle] = &resource;

    return buffer;
}

void ResourceManager::referenceTexture(gerium_texture_h handle) {
    referenceResource(Texture + handle.unused);
}

void ResourceManager::referenceTechnique(gerium_technique_h handle) {
    referenceResource(Technique + handle.unused);
}

void ResourceManager::referenceBuffer(gerium_buffer_h handle) {
    referenceResource(Buffer + handle.unused);
}

void ResourceManager::deleteTexture(gerium_texture_h handle) {
    deleteResource(Texture + handle.unused);
}

void ResourceManager::deleteTechnique(gerium_technique_h handle) {
    deleteResource(Technique + handle.unused);
}

void ResourceManager::deleteBuffer(gerium_buffer_h handle) {
    deleteResource(Buffer + handle.unused);
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

gerium_uint32_t ResourceManager::_bufferCount = 0;
