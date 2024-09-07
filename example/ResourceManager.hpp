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
    static constexpr gerium_uint64_t NoRetention      = 0;
    static constexpr gerium_uint64_t DefaultRetention = 240'000;

    void create(AsyncLoader& loader, gerium_frame_graph_t frameGraph);
    void destroy();
    void update(gerium_uint64_t elapsedMs);

    void loadFrameGraph(const std::string& path);
    Texture loadTexture(const std::string& path, gerium_uint64_t retentionMs = DefaultRetention);
    Technique loadTechnique(const std::string& path, gerium_uint64_t retentionMs = DefaultRetention);

    Texture createTexture(const gerium_texture_info_t& info,
                          gerium_cdata_t data,
                          gerium_uint64_t retentionMs = DefaultRetention);
    Technique createTechnique(const std::string& name,
                              const std::vector<gerium_pipeline_t>& pipelines,
                              gerium_uint64_t retentionMs = DefaultRetention);
    Buffer createBuffer(gerium_buffer_usage_flags_t bufferUsage,
                        gerium_bool_t dynamic,
                        const std::string& name,
                        gerium_cdata_t data,
                        gerium_uint32_t size,
                        gerium_uint64_t retentionMs = DefaultRetention);
    DescriptorSet createDescriptorSet(const std::string& name,
                                      bool global                 = false,
                                      gerium_uint64_t retentionMs = NoRetention);

    Texture getTextureByPath(const std::string& path);
    Texture getTextureByName(const std::string& name);
    Technique getTechniqueByPath(const std::string& path);
    Technique getTechniqueByName(const std::string& name);
    Buffer getBufferByPath(const std::string& path);
    Buffer getBufferByName(const std::string& name);
    DescriptorSet getDescriptorSetByName(const std::string& name);

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
        TextureType       = 1,
        TechniqueType     = 2,
        BufferType        = 3,
        DescriptorSetType = 4
    };

    struct Resource {
        Type type;
        gerium_uint16_t handle;
        gerium_uint16_t reference;
        gerium_uint64_t path;
        gerium_uint64_t name;
        gerium_uint64_t lastTick;
        gerium_uint64_t retentionMs;
    };

    template <typename>
    struct HandleToType {};

    template <>
    struct HandleToType<gerium_texture_h> {
        static constexpr Type type = TextureType;
    };

    template <>
    struct HandleToType<gerium_technique_h> {
        static constexpr Type type = TechniqueType;
    };

    template <>
    struct HandleToType<gerium_buffer_h> {
        static constexpr Type type = BufferType;
    };

    template <>
    struct HandleToType<gerium_descriptor_set_h> {
        static constexpr Type type = DescriptorSetType;
    };

    template <typename H>
    ::Resource<H> getResourceByPath(const std::string& path) noexcept {
        if (path.length()) {
            if (auto it = _pathes.find(calcKey(path, HandleToType<H>::type)); it != _pathes.end()) {
                return { this, H{ it->second->handle } };
            }
        }
        return nullptr;
    }

    template <typename H>
    ::Resource<H> getResourceByName(const std::string& name) noexcept {
        if (name.length()) {
            if (auto it = _names.find(calcKey(name, HandleToType<H>::type)); it != _names.end()) {
                return { this, H{ it->second->handle } };
            }
        }
        return nullptr;
    }

    template <typename H>
    void addResource(const std::string& path, const std::string& name, H handle, gerium_uint64_t retentionMs) {
        auto key = calcHandleKey(handle);

        auto& resource       = _resources[key];
        resource.type        = HandleToType<H>::type;
        resource.handle      = handle.index;
        resource.reference   = 0;
        resource.path        = 0;
        resource.name        = 0;
        resource.lastTick    = _ticks;
        resource.retentionMs = retentionMs;

        if (auto key = calcKey(path, HandleToType<H>::type)) {
            resource.path = key;
            _pathes[key]  = &resource;
        }
        if (auto key = calcKey(name, HandleToType<H>::type)) {
            resource.name = key;
            _names[key]   = &resource;
        }
    }

    template <typename H>
    void reference(const ::Resource<H>& ref) noexcept {
        const H handle = ref;
        if (auto it = _resources.find(calcHandleKey(handle)); it != _resources.end()) {
            ++it->second.reference;
        }
    }

    template <typename H>
    void destroy(const ::Resource<H>& ref) noexcept {
        const H handle = ref;
        if (auto it = _resources.find(calcHandleKey(handle)); it != _resources.end()) {
            --it->second.reference;
            it->second.lastTick = _ticks;
        }
    }

    template <typename H>
    static gerium_uint32_t calcHandleKey(H handle) noexcept {
        auto type = ((gerium_uint32_t) HandleToType<H>::type) << 16;
        return type | gerium_uint32_t(handle.index);
    }

    static gerium_uint64_t calcKey(const std::string& str, Type type) noexcept;

    gerium_renderer_t _renderer{};
    gerium_frame_graph_t _frameGraph{};
    AsyncLoader* _loader{};
    gerium_uint64_t _ticks{};
    std::map<gerium_uint32_t, Resource> _resources;
    std::map<gerium_uint64_t, Resource*> _pathes;
    std::map<gerium_uint64_t, Resource*> _names;
};

template <typename T>
inline Resource<T>::Resource(std::nullptr_t) noexcept : Resource() {
}

template <typename T>
inline Resource<T>::Resource(ResourceManager* resourceManager, T handle) noexcept :
    _resourceManager(resourceManager),
    _handle(handle) {
    if (resourceManager) {
        _resourceManager->reference(*this);
    }
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
        _resourceManager->reference(*this);
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
            _resourceManager->reference(*this);
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
        _resourceManager->destroy(*this);
        _resourceManager = nullptr;
        _handle          = { UndefinedHandle };
    }
}

#endif
