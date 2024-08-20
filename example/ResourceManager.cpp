#include "ResourceManager.hpp"

void ResourceManager::create(AsyncLoader& loader) {
    _renderer = loader.renderer();
    _loader   = &loader;
}

void ResourceManager::destroy() {
    std::vector<std::string> keys;
    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if (it->second.reference == 0) {
            gerium_renderer_destroy_texture(_renderer, it->second.handle);
            keys.push_back(it->first);
        }
    }
    for (const auto& key : keys) {
        _textures.erase(key);
    }
}

void ResourceManager::update(gerium_float32_t elapsed) {
    _ticks += elapsed;

    std::vector<std::string> keys;
    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if (it->second.reference == 0) {
            if (_ticks - it->second.lastUsed > 240000.0) {
                gerium_renderer_destroy_texture(_renderer, it->second.handle);
                keys.push_back(it->first);
            }
        }
    }
    for (const auto& key : keys) {
        _textures.erase(key);
    }
}

gerium_texture_h ResourceManager::loadTexture(const std::filesystem::path& path) {
    auto pathStr = path.string();
    if (auto it = _textures.find(pathStr); it != _textures.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
        return it->second.handle;
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

    TextureResource resource{};
    resource.handle    = texture;
    resource.reference = 1;
    resource.lastUsed  = _ticks;

    _textures[path.string()] = resource;
    return texture;
}

void ResourceManager::referenceTexture(gerium_texture_h handle) {
    if (handle.unused == UndefinedHandle) {
        return;
    }
    auto it = _textures.begin();
    for (; it != _textures.end(); ++it) {
        if (it->second.handle.unused == handle.unused) {
            break;
        }
    }
    if (it != _textures.end()) {
        ++it->second.reference;
        it->second.lastUsed = _ticks;
    }
}

void ResourceManager::deleteTexture(gerium_texture_h handle) {
    if (handle.unused == UndefinedHandle) {
        return;
    }
    auto it = _textures.begin();
    for (; it != _textures.end(); ++it) {
        if (it->second.handle.unused == handle.unused) {
            break;
        }
    }
    if (it != _textures.end()) {
        --it->second.reference;
        it->second.lastUsed = _ticks;
    }
}
