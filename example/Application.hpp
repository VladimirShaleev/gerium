#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "ResourceManager.hpp"

struct Settings {
    bool DrawBBox;
    bool Camera2;
};

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

private:
    void addPass(RenderPass& renderPass);
    void createScene();

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
    GBufferPass _gbufferPass{};
    PresentPass _presentPass{};
    std::vector<RenderPass*> _renderPasses{};

    AsyncLoader _asyncLoader{};
    ResourceManager _resourceManager{};
    Camera _camera{};

    gerium_uint16_t _prevWidth{};
    gerium_uint16_t _prevHeight{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
    gerium_float32_t _invWidth{};
    gerium_float32_t _invHeight{};

    Technique _baseTechnique{};
};

#endif
