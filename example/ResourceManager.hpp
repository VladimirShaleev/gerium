#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include "AsyncLoader.hpp"

class ResourceManager final {
public:
    void create(AsyncLoader& loader, gerium_frame_graph_t frameGraph);
    void destroy();
    void update(gerium_float32_t elapsed);

    gerium_texture_h loadTexture(const std::filesystem::path& path);
    gerium_technique_h loadTechnique(const std::string& name);
    gerium_buffer_h loadBuffer(const std::string& path, const std::string& name);
    gerium_technique_h createTechnique(const std::string& name, const std::vector<gerium_pipeline_t> pipelines);
    gerium_buffer_h createBuffer(gerium_buffer_usage_flags_t bufferUsage,
                                 gerium_bool_t dynamic,
                                 const std::string& path,
                                 const std::string& name,
                                 gerium_cdata_t data,
                                 gerium_uint32_t size);

    void referenceTexture(gerium_texture_h handle);
    void referenceTechnique(gerium_technique_h handle);
    void referenceBuffer(gerium_buffer_h handle);

    void deleteTexture(gerium_texture_h handle);
    void deleteTechnique(gerium_technique_h handle);
    void deleteBuffer(gerium_buffer_h handle);

    gerium_renderer_t renderer() noexcept {
        return _renderer;
    }

    gerium_frame_graph_t frameGraph() noexcept {
        return _frameGraph;
    }

private:
    enum Type {
        Texture   = 0,
        Technique = 10000,
        Buffer    = 20000
    };

    struct Resource {
        Type type;
        std::string name;
        gerium_uint64_t key;
        gerium_uint16_t handle;
        gerium_uint16_t reference;
        gerium_float64_t lastUsed;
    };

    void referenceResource(gerium_uint16_t handle);
    void deleteResource(gerium_uint16_t handle);

    static gerium_uint64_t calcKey(const std::string& path) noexcept;

    gerium_renderer_t _renderer{};
    gerium_frame_graph_t _frameGraph{};
    AsyncLoader* _loader{};
    gerium_float64_t _ticks{};
    std::map<gerium_uint64_t, Resource> _resources;
    std::map<gerium_uint16_t, Resource*> _mapResource;

    static gerium_uint32_t _bufferCount;
};

#endif
