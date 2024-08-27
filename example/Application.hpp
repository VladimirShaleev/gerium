#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "RenderPass.hpp"
#include "Scene.hpp"

class GBufferPass final : public RenderPass {
public:
    GBufferPass() : RenderPass("gbuffer_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
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

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

    static bool drawBBox() noexcept {
        return _drawBBox;
    }

    static bool camera2() noexcept {
        return _camera2;
    }

private:
    Technique _technique{};
    DescriptorSet _descriptorSet{};
    static bool _drawBBox;
    static bool _camera2;
};

class DepthPrePass final : public RenderPass {
public:
    DepthPrePass() : RenderPass("depth_pre_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
};

class LightPass final : public RenderPass {
public:
    LightPass() : RenderPass("light_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
             
    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
   
private:
    DescriptorSet _descriptorSet{};

    int _maxPoints{};
    Buffer _vertices{};
    Technique _lines{};
};

class Application final {
public:
    Application();
    ~Application();

    void run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height);

    Scene& scene() noexcept {
        return _scene;
    }

    ResourceManager& resourceManager() noexcept {
        return _resourceManager;
    }

private:
    void addPass(RenderPass& renderPass);
    void createScene();

    void initialize();
    void uninitialize();

    void pollInput(gerium_float32_t elapsed);
    void frame(gerium_float32_t elapsed);
    void state(gerium_application_state_t state);

    static gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_float32_t elapsed);
    static gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state);

    static gerium_uint32_t prepare(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data);
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

    GBufferPass _gbufferPass{};
    PresentPass _presentPass{};
    DepthPrePass _depthPrePass{};
    LightPass _lightPass{};
    std::vector<RenderPass*> _renderPasses{};

    AsyncLoader _asyncLoader{};
    ResourceManager _resourceManager{};
    Scene _scene{};

    Technique _baseTechnique{};
};

#endif
