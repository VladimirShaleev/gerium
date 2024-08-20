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
    enum Type {
        Texture
    };

    struct Resource {
        Type type;
        std::string path;
        gerium_uint64_t key;
        gerium_uint16_t handle;
        gerium_uint16_t reference;
        gerium_float64_t lastUsed;
    };

    void referenceResource(gerium_uint16_t handle);
    void deleteResource(gerium_uint16_t handle);

    static gerium_uint64_t calcKey(const std::string& path) noexcept;

    gerium_renderer_t _renderer{};
    AsyncLoader* _loader{};
    gerium_float64_t _ticks{};
    std::map<gerium_uint64_t, Resource> _resources;
    std::map<gerium_uint16_t, Resource*> _mapResource;
};

#endif
