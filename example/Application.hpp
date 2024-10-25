#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "ResourceManager.hpp"

struct Settings {
    bool DrawBBox;
    bool DebugCamera;
    bool MoveDebugCamera;
};

struct ClusterDatas {
    std::vector<VertexOptimized> vertices;
    std::vector<MeshletOptimized> meshlets;
    std::vector<ClusterMesh> meshes;
    std::vector<uint32_t> vertexIndices;
    std::vector<uint8_t> primitiveIndices;

    Buffer verticesBuffer;
    Buffer meshletsBuffer;
    Buffer meshesBuffer;
    Buffer vertexIndicesBuffer;
    Buffer primitiveIndicesBuffer;
};

class DepthPyramidPass final : public RenderPass {
public:
    DepthPyramidPass() : RenderPass("depth_pyramid_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    void createDepthPyramid(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer);

    gerium_uint16_t _depthPyramidWidth{};
    gerium_uint16_t _depthPyramidHeight{};
    gerium_uint16_t _depthPyramidMipLevels{};
    Texture _depthPyramid{};
    Texture _depthPyramidMips[16]{};
    Buffer _imageSizes[16]{};
    DescriptorSet _descriptorSets[16]{};
};

class CullingPass final : public RenderPass {
public:
    CullingPass(bool late) : RenderPass(!late ? "culling_pass" : "culling_late_pass"), _latePass(late) {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet0{};
    DescriptorSet _descriptorSet1{};
    bool _latePass{};
};

class IndirectPass final : public RenderPass {
public:
    IndirectPass(bool late) : RenderPass(!late ? "indirect_pass" : "indirect_late_pass"), _latePass(late) {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet;
    bool _latePass{};
};

class GBufferPass final : public RenderPass {
public:
    GBufferPass(bool late) : RenderPass(!late ? "gbuffer_pass" : "gbuffer_late_pass"), _latePass(late) {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet;
    bool _latePass{};
};

class PresentPass final : public RenderPass {
public:
    PresentPass() : RenderPass("present_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
};

class DebugOcclusionPass final : public RenderPass {
public:
    DebugOcclusionPass() : RenderPass("debug_occlusion_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet;
};

class DebugLinePass final : public RenderPass {
public:
    DebugLinePass() : RenderPass("debug_line_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    int _maxPoints{};
    DescriptorSet _descriptorSet{};
    Buffer _vertices{};
};

class Application final {
public:
    Application();
    ~Application();

    void run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);

    ResourceManager& resourceManager() noexcept {
        return _resourceManager;
    }

    Settings& settings() noexcept {
        return _settings;
    }

    bool bindlessSupported() const noexcept {
        return _bindlessSupported;
    }

    bool meshShaderSupported() const noexcept {
        return _meshShaderSupported;
    }

    Camera* getCamera() noexcept {
        return &_camera;
    }

    Camera* getDebugCamera() noexcept {
        return &_debugCamera;
    }

    gerium_uint16_t width() const noexcept {
        return _width;
    }

    gerium_uint16_t height() const noexcept {
        return _height;
    }

    gerium_float32_t invWidth() const noexcept {
        return _invWidth;
    }

    gerium_float32_t invHeight() const noexcept {
        return _invHeight;
    }

    gerium_technique_h getBaseTechnique() const noexcept {
        return _baseTechnique;
    }

    ClusterDatas& clusterDatas() noexcept {
        return _clusterDatas;
    }

    gerium_buffer_h drawData() const noexcept {
        return _drawData;
    }

    gerium_buffer_h instances() const noexcept {
        return _instancesBuffer;
    }

    gerium_uint32_t instanceCount() const noexcept {
        return gerium_uint32_t(_instances.size());
    }

private:
    void addPass(RenderPass& renderPass);
    void createScene();
    void uploadClusterDatas(ClusterDatas& clusterDatas, gerium_uint32_t id);
    ClusterMeshInstance loadClusterMesh(ClusterDatas& clusterDatas, std::string_view name) const;
    size_t appendMeshlets(ClusterDatas& clusterDatas,
                          const VertexOptimized* vertices,
                          size_t verticesOffset,
                          size_t verticesCount,
                          const std::vector<uint32_t>& indices) const;

    void initialize();
    void uninitialize();

    void pollInput(gerium_uint64_t elapsedMs);
    void frame(gerium_uint64_t elapsedMs);
    void state(gerium_application_state_t state);

    static gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_uint64_t elapsedMs);
    static gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state);

    static gerium_uint32_t prepare(gerium_frame_graph_t frameGraph,
                                   gerium_renderer_t renderer,
                                   gerium_uint32_t maxWorkers,
                                   gerium_data_t data);
    static gerium_bool_t resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data);
    static gerium_bool_t render(gerium_frame_graph_t frameGraph,
                                gerium_renderer_t renderer,
                                gerium_command_buffer_t commandBuffer,
                                gerium_uint32_t worker,
                                gerium_uint32_t totalWorkers,
                                gerium_data_t data);

    template <typename Func>
    bool cppCall(Func&& func) noexcept {
        try {
            func();
            return true;
        } catch (...) {
            _error = std::current_exception();
            return false;
        }
    }

    template <typename Func>
    gerium_uint32_t cppCallInt(Func&& func) noexcept {
        try {
            return func();
        } catch (...) {
            _error = std::current_exception();
            return 0;
        }
    }

    std::exception_ptr _error{};
    gerium_logger_t _logger{};
    gerium_application_t _application{};
    gerium_renderer_t _renderer{};
    gerium_profiler_t _profiler{};
    gerium_frame_graph_t _frameGraph{};
    bool _bindlessSupported{};
    bool _meshShaderSupported{};

    Settings _settings{};
    DepthPyramidPass _depthPyramidPass{};
    CullingPass _cullingPass{ false };
    CullingPass _cullingLatePass{ true };
    GBufferPass _gbufferPass{ false };
    GBufferPass _gbufferLatePass{ true };
    PresentPass _presentPass{};
    IndirectPass _indirectPass{ false };
    IndirectPass _indirectLatePass{ true };
    DebugOcclusionPass _debugOcclusionPass;
    DebugLinePass _debugLinePass{};
    std::vector<RenderPass*> _renderPasses{};

    AsyncLoader _asyncLoader{};
    ResourceManager _resourceManager{};
    Camera _camera{};
    Camera _debugCamera{};

    gerium_uint16_t _prevWidth{};
    gerium_uint16_t _prevHeight{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
    gerium_float32_t _invWidth{};
    gerium_float32_t _invHeight{};

    Technique _baseTechnique{};

    ClusterDatas _clusterDatas{};
    std::vector<ClusterMeshInstance> _instances{};
    Buffer _drawData{};
    Buffer _instancesBuffer{};
};

#endif
