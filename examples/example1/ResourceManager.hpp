#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "Common.hpp"

class ResourceManager;

template <typename H>
class Resource final {
public:
    Resource() = default;
    Resource(std::nullptr_t) noexcept;
    Resource(ResourceManager* resourceManager, H handle) noexcept;
    ~Resource();

    Resource(const Resource& resource) noexcept;
    Resource(Resource&& resource) noexcept;

    Resource& operator=(std::nullptr_t) noexcept;
    Resource& operator=(const Resource& resource) noexcept;
    Resource& operator=(Resource&& resource) noexcept;

    operator H() noexcept;

    operator const H() const noexcept;

    explicit operator bool() const noexcept;

    bool operator==(const Resource& rhs) const noexcept;
    bool operator!=(const Resource& rhs) const noexcept;
    auto operator<=>(const Resource& rhs) const noexcept;

private:
    void destroy() noexcept;

    ResourceManager* _resourceManager{};
    H _handle{ UndefinedHandle };
};

using Buffer        = Resource<gerium_buffer_h>;
using Texture       = Resource<gerium_texture_h>;
using Technique     = Resource<gerium_technique_h>;
using DescriptorSet = Resource<gerium_descriptor_set_h>;

class ResourceManager final {
public:
    static constexpr gerium_uint64_t NoRetention      = 0;
    static constexpr gerium_uint64_t DefaultRetention = 240'000;

    void create(gerium_renderer_t renderer, gerium_frame_graph_t frameGraph);
    void destroy();
    void update(gerium_uint64_t elapsedMs);

    void loadFrameGraph(const entt::hashed_string& name);
    Texture loadTexture(const entt::hashed_string& path, gerium_uint64_t retentionMs = DefaultRetention);
    Technique loadTechnique(const entt::hashed_string& name, gerium_uint64_t retentionMs = DefaultRetention);

    Texture createTexture(const gerium_texture_info_t& info,
                          gerium_cdata_t data,
                          gerium_uint64_t retentionMs = DefaultRetention);
    Texture createTextureView(const entt::hashed_string& name,
                              const Texture& texture,
                              gerium_texture_type_t type,
                              gerium_uint16_t mipBaseLevel,
                              gerium_uint16_t mipLevelCount,
                              gerium_uint16_t layerBase   = 0,
                              gerium_uint16_t layerCount  = 1,
                              gerium_uint64_t retentionMs = DefaultRetention);
    Technique createTechnique(const entt::hashed_string& name,
                              const std::vector<gerium_pipeline_t>& pipelines,
                              gerium_uint64_t retentionMs = DefaultRetention);
    Buffer createBuffer(gerium_buffer_usage_flags_t bufferUsage,
                        gerium_bool_t dynamic,
                        const entt::hashed_string& name,
                        gerium_cdata_t data,
                        gerium_uint32_t size,
                        gerium_uint64_t retentionMs = DefaultRetention);
    DescriptorSet createDescriptorSet(const entt::hashed_string& name,
                                      bool global                 = false,
                                      gerium_uint64_t retentionMs = NoRetention);

    Texture getTextureByPath(const entt::hashed_string& path) noexcept;
    Texture getTextureByName(const entt::hashed_string& name) noexcept;
    Technique getTechniqueByPath(const entt::hashed_string& path) noexcept;
    Technique getTechniqueByName(const entt::hashed_string& name) noexcept;
    Buffer getBufferByPath(const entt::hashed_string& path) noexcept;
    Buffer getBufferByName(const entt::hashed_string& name) noexcept;
    DescriptorSet getDescriptorSetByName(const entt::hashed_string& name) noexcept;

    gerium_renderer_t renderer() noexcept {
        return _renderer;
    }

    gerium_frame_graph_t frameGraph() noexcept {
        return _frameGraph;
    }

private:
    template <typename H>
    friend class Resource;

    struct ResourceData {
        gerium_uint32_t type;
        gerium_uint16_t handle;
        gerium_uint16_t references;
        gerium_uint64_t pathKey;
        gerium_uint64_t nameKey;
        gerium_uint64_t lastTick;
        gerium_uint64_t retentionMs;
    };

    template <typename H>
    static gerium_uint32_t getHandleId() noexcept {
        return entt::type_index<H>::value();
    }

    template <typename H>
    Resource<H> getResourceByPath(const entt::hashed_string& path) noexcept {
        if (path.size()) {
            if (auto it = _pathes.find(calcKey<H>(path)); it != _pathes.end()) {
                return { this, H{ it->second->handle } };
            }
        }
        return nullptr;
    }

    template <typename H>
    Resource<H> getResourceByName(const entt::hashed_string& name) noexcept {
        if (name.size()) {
            if (auto it = _names.find(calcKey<H>(name)); it != _names.end()) {
                return { this, H{ it->second->handle } };
            }
        }
        return nullptr;
    }

    template <typename H>
    void addResource(const entt::hashed_string& path,
                     const entt::hashed_string& name,
                     H handle,
                     gerium_uint64_t retentionMs) {
        auto& resource       = _resources[calcHandleKey(handle)];
        resource.type        = getHandleId<H>();
        resource.handle      = handle.index;
        resource.references  = 0;
        resource.pathKey     = 0;
        resource.nameKey     = 0;
        resource.lastTick    = _ticks;
        resource.retentionMs = 0;

        bool hasRetention = false;
        if (auto key = calcKey<H>(path)) {
            resource.pathKey = key;
            _pathes[key]     = &resource;
            hasRetention     = true;
        }
        if (auto key = calcKey<H>(name)) {
            resource.nameKey = key;
            _names[key]      = &resource;
            hasRetention     = true;
        }
        if (hasRetention) {
            resource.retentionMs = retentionMs;
        }
    }

    template <typename H>
    void addReference(const Resource<H>& ref, gerium_sint16_t add) noexcept {
        const H handle = ref;
        if (auto it = _resources.find(calcHandleKey(handle)); it != _resources.end()) {
            it->second.references = gerium_sint16_t(it->second.references) + add;
        }
    }

    template <typename H>
    void reference(const Resource<H>& ref) noexcept {
        addReference(ref, 1);
    }

    template <typename H>
    void destroy(const Resource<H>& ref) noexcept {
        addReference(ref, -1);
    }

    template <typename H>
    static gerium_uint32_t calcHandleKey(H handle) noexcept {
        const auto handleKey = getHandleId<H>();
        return (gerium_uint32_t(handleKey) << 16) | gerium_uint32_t(handle.index);
    }

    template <typename H>
    static gerium_uint64_t calcKey(const entt::hashed_string& str) noexcept {
        if (str.size()) {
            const auto handleKey = getHandleId<H>();
            return ((gerium_uint64_t(handleKey)) << 32) | gerium_uint64_t(str.value());
        }
        return 0;
    }

    static std::string calcFrameGraphPath(const entt::hashed_string& name) noexcept;
    static std::string calcTexturePath(const entt::hashed_string& filename) noexcept;
    static std::string calcTechniquePath(const entt::hashed_string& filename) noexcept;

    gerium_renderer_t _renderer{};
    gerium_frame_graph_t _frameGraph{};
    gerium_uint64_t _ticks{};
    std::map<gerium_uint32_t, ResourceData> _resources;
    std::map<gerium_uint64_t, ResourceData*> _pathes;
    std::map<gerium_uint64_t, ResourceData*> _names;
};

template <typename H>
inline Resource<H>::Resource(std::nullptr_t) noexcept : Resource() {
}

template <typename H>
inline Resource<H>::Resource(ResourceManager* resourceManager, H handle) noexcept :
    _resourceManager(resourceManager),
    _handle(handle) {
    if (resourceManager) {
        _resourceManager->reference(*this);
    }
}

template <typename H>
inline Resource<H>::~Resource() {
    destroy();
}

template <typename H>
inline Resource<H>::Resource(const Resource& resource) noexcept :
    _resourceManager(resource._resourceManager),
    _handle(resource._handle) {
    if (_resourceManager) {
        _resourceManager->reference(*this);
    }
}

template <typename H>
inline Resource<H>::Resource(Resource&& resource) noexcept :
    _resourceManager(resource._resourceManager),
    _handle(resource._handle) {
    resource._resourceManager = nullptr;
    resource._handle          = { UndefinedHandle };
}

template <typename H>
inline Resource<H>& Resource<H>::operator=(std::nullptr_t) noexcept {
    destroy();
    return *this;
}

template <typename H>
inline Resource<H>& Resource<H>::operator=(const Resource<H>& resource) noexcept {
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

template <typename H>
inline Resource<H>& Resource<H>::operator=(Resource<H>&& resource) noexcept {
    if (_handle.index != resource._handle.index) {
        destroy();
        std::swap(_resourceManager, resource._resourceManager);
        std::swap(_handle, resource._handle);
    }
    return *this;
}

template <typename H>
inline Resource<H>::operator H() noexcept {
    return _handle;
}

template <typename H>
inline Resource<H>::operator const H() const noexcept {
    return _handle;
}

template <typename H>
inline Resource<H>::operator bool() const noexcept {
    return _resourceManager && _handle.index != UndefinedHandle;
}

template <typename H>
inline bool Resource<H>::operator==(const Resource& rhs) const noexcept {
    return _handle.index == rhs._handle.index;
}

template <typename H>
inline bool Resource<H>::operator!=(const Resource& rhs) const noexcept {
    return !(*this == rhs);
}

template <typename H>
inline auto Resource<H>::operator<=>(const Resource& rhs) const noexcept {
    return _handle.index <=> rhs._handle.index;
}

template <typename H>
inline void Resource<H>::destroy() noexcept {
    if (_resourceManager) {
        _resourceManager->destroy(*this);
        _resourceManager = nullptr;
        _handle          = { UndefinedHandle };
    }
}

namespace std {

template <typename H>
struct hash<Resource<H>> {
    size_t operator()(const Resource<H>& resource) const {
        return H(resource).index;
    }
};

} // namespace std

#endif
