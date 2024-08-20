#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "AsyncLoader.hpp"

class ResourceManager final {
public:
    void create(AsyncLoader& loader);
    void destroy();
    void update(gerium_float32_t elapsed);

    gerium_texture_h loadTexture(const std::filesystem::path& path);
    void referenceTexture(gerium_texture_h handle);
    void deleteTexture(gerium_texture_h handle);

private:
    struct TextureResource {
        gerium_texture_h handle;
        gerium_uint16_t reference;
        gerium_float64_t lastUsed;
    };

    gerium_renderer_t _renderer{}; 
    AsyncLoader* _loader{};
    gerium_float64_t _ticks{};
    std::map<std::string, TextureResource> _textures;
};

#endif
