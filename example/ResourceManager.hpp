#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "AsyncLoader.hpp"

class ResourceManager;

template <typename T>
class Resource final {
public:
    Resource() = default;
    Resource(std::nullptr_t) noexcept;
    Resource(ResourceManager* resourceManager, T handle) noexcept;
    ~Resource();

    Resource(const Resource& resource) noexcept;
    Resource(Resource&& resource) noexcept;

    Resource& operator=(std::nullptr_t) noexcept;
    Resource& operator=(const Resource& resource) noexcept;
    Resource& operator=(Resource&& resource) noexcept;

    operator T() noexcept;

    operator const T() const noexcept;

    explicit operator bool() const noexcept;

private:
    void destroy() noexcept;

    ResourceManager* _resourceManager{};
    T _handle{ UndefinedHandle };
};

using Buffer        = Resource<gerium_buffer_h>;
using Texture       = Resource<gerium_texture_h>;
using Technique     = Resource<gerium_technique_h>;
using DescriptorSet = Resource<gerium_descriptor_set_h>;

class ResourceManager final {
public:
    void create(AsyncLoader& loader, gerium_frame_graph_t frameGraph);
    void destroy();
    void update(gerium_float32_t elapsed);

    void loadFrameGraph(const std::filesystem::path& path);
    Texture loadTexture(const std::filesystem::path& path);
    Technique loadTechnique(const std::filesystem::path& path);
    Technique getTechnique(const std::string& name);
    Buffer getBuffer(const std::string& path, const std::string& name);
    Texture createTexture(const gerium_texture_info_t& info, gerium_cdata_t data);
    Technique createTechnique(const std::string& name, const std::vector<gerium_pipeline_t>& pipelines);
    Buffer createBuffer(gerium_buffer_usage_flags_t bufferUsage,
                        gerium_bool_t dynamic,
                        const std::string& path,
                        const std::string& name,
                        gerium_cdata_t data,
                        gerium_uint32_t size);
    DescriptorSet createDescriptorSet(bool global = false);

    gerium_renderer_t renderer() noexcept {
        return _renderer;
    }

    gerium_frame_graph_t frameGraph() noexcept {
        return _frameGraph;
    }

private:
    template <typename T>
    friend class Resource;

    enum Type {
        TextureType       = 0,
        TechniqueType     = 10000,
        BufferType        = 20000,
        DescriptorSetType = 30000
    };

    struct Resource {
        Type type;
        std::string name;
        gerium_uint64_t key;
        gerium_uint16_t handle;
        gerium_uint16_t reference;
        gerium_float64_t lastUsed;
    };

    void reference(gerium_texture_h handle);
    void reference(gerium_technique_h handle);
    void reference(gerium_buffer_h handle);
    void reference(gerium_descriptor_set_h handle);

    void destroy(gerium_texture_h handle);
    void destroy(gerium_technique_h handle);
    void destroy(gerium_buffer_h handle);
    void destroy(gerium_descriptor_set_h handle);

    void referenceResource(gerium_uint16_t handle);
    void destroyResource(gerium_uint16_t handle);

    static gerium_uint64_t calcKey(const std::string& path) noexcept;

    gerium_renderer_t _renderer{};
    gerium_frame_graph_t _frameGraph{};
    AsyncLoader* _loader{};
    gerium_float64_t _ticks{};
    std::map<gerium_uint64_t, Resource> _resources;
    std::map<gerium_uint16_t, Resource*> _mapResources;

    static gerium_uint32_t _resourceCount;
};

template <typename T>
inline Resource<T>::Resource(std::nullptr_t) noexcept : Resource() {
}

template <typename T>
inline Resource<T>::Resource(ResourceManager* resourceManager, T handle) noexcept :
    _resourceManager(resourceManager),
    _handle(handle) {
}

template <typename T>
inline Resource<T>::~Resource() {
    destroy();
}

template <typename T>
inline Resource<T>::Resource(const Resource& resource) noexcept :
    _resourceManager(resource._resourceManager),
    _handle(resource._handle) {
    if (_resourceManager) {
        _resourceManager->reference(_handle);
    }
}

template <typename T>
inline Resource<T>::Resource(Resource&& resource) noexcept :
    _resourceManager(resource._resourceManager),
    _handle(resource._handle) {
    resource._resourceManager = nullptr;
    resource._handle          = { UndefinedHandle };
}

template <typename T>
inline Resource<T>& Resource<T>::operator=(std::nullptr_t) noexcept {
    destroy();
    return *this;
}

template <typename T>
inline Resource<T>& Resource<T>::operator=(const Resource<T>& resource) noexcept {
    if (_handle.index != resource._handle.index) {
        destroy();
        _resourceManager = resource._resourceManager;
        _handle          = resource._handle;
        if (_resourceManager) {
            _resourceManager->reference(_handle);
        }
    }
    return *this;
}

template <typename T>
inline Resource<T>& Resource<T>::operator=(Resource<T>&& resource) noexcept {
    if (_handle.index != resource._handle.index) {
        destroy();
        std::swap(_resourceManager, resource._resourceManager);
        std::swap(_handle, resource._handle);
    }
    return *this;
}

template <typename T>
inline Resource<T>::operator T() noexcept {
    return _handle;
}

template <typename T>
inline Resource<T>::operator const T() const noexcept {
    return _handle;
}

template <typename T>
inline Resource<T>::operator bool() const noexcept {
    return _resourceManager && _handle.index != UndefinedHandle;
}

template <typename T>
inline void Resource<T>::destroy() noexcept {
    if (_resourceManager) {
        _resourceManager->destroy(_handle);
        _resourceManager = nullptr;
        _handle          = { UndefinedHandle };
    }
}

#endif
