#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "RenderPass.hpp"
#include "Scene.hpp"

struct Settings {
    bool DrawBBox;
    bool Camera2;
};

class GBufferPass final : public RenderPass {
public:
    GBufferPass() : RenderPass("gbuffer_pass") {
    }

    gerium_uint32_t prepare(gerium_frame_graph_t frameGraph,
                            gerium_renderer_t renderer,
                            gerium_uint32_t maxWorkers) override;

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    std::array<DescriptorSet, 4> _descriptorSets{};
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

private:
    Technique _technique{};
    DescriptorSet _descriptorSet{};
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
    void resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    DescriptorSet _descriptorSet{};

    int _maxPoints{};
    Buffer _vertices{};
    Technique _lines{};
};

class TAAPass final : public RenderPass {
public:
    TAAPass() : RenderPass("taa_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

    void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;
    void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) override;

private:
    Technique _technique{};
    DescriptorSet _descriptorSet{};
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

    Settings& settings() noexcept {
        return _settings;
    }

    bool bindlessSupported() const noexcept {
        return _bindlessSupported;
    }

    Camera* getCamera2() noexcept {
        return _camera2;
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

    const glm::vec2& previousJitter() const noexcept {
        return _previousJitter;
    }

    const glm::vec2& currentJitter() const noexcept {
        return _jitterTable[_jitterIndex];
    }

private:
    template <typename RP, typename... Args>
    void addPass(Args&&... args) {
        static_assert(std::is_base_of_v<RenderPass, RP>);

        auto renderPass = std::make_unique<RP>(std::forward<Args>(args)...);
        renderPass->setApplication(this);

        gerium_render_pass_t pass{ prepare, resize, render };
        gerium_frame_graph_add_pass(_frameGraph, renderPass->name().c_str(), &pass, renderPass.get());

        _renderPassesCache[typeId<RP>] = renderPass.get();
        _renderPasses.push_back(std::move(renderPass));
    }

    void createScene();

    void initialize();
    void uninitialize();

    void pollInput(gerium_uint64_t elapsedMs);
    void updateJitterTable();
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

    // We wrap the call into a C++ member function so that we can propagate the exception,
    // if it occurs, back to the C++ code and rethrow it after the C API callback has done
    template <typename Func>
    std::conditional_t<std::is_same_v<std::invoke_result_t<Func>, void>, bool, std::invoke_result_t<Func>>
    cppCallWrap(Func&& func) noexcept {
        using ReturnType = std::invoke_result_t<Func>;

        constexpr auto isVoidReturn = std::is_same_v<ReturnType, void>;
        try {
            if constexpr (isVoidReturn) {
                func();
                return true;
            } else {
                return func();
            }
        } catch (...) {
            uninitialize();
            _error = std::current_exception();
            if constexpr (isVoidReturn) {
                return false;
            } else {
                return ReturnType();
            }
        }
    }

    std::exception_ptr _error{};
    gerium_logger_t _logger{};
    gerium_application_t _application{};
    gerium_renderer_t _renderer{};
    gerium_profiler_t _profiler{};
    gerium_frame_graph_t _frameGraph{};
    bool _bindlessSupported{};

    Settings _settings{};
    std::vector<std::unique_ptr<RenderPass>> _renderPasses{};
    std::map<int, RenderPass*> _renderPassesCache{};

    ResourceManager _resourceManager{};
    Scene _scene{};
    Camera* _camera2{};

    gerium_uint16_t _prevWidth{};
    gerium_uint16_t _prevHeight{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
    gerium_float32_t _invWidth{};
    gerium_float32_t _invHeight{};
    gerium_sint32_t _jitterIndex{ 127 };
    gerium_sint32_t _jitterPeriod{ 128 };
    std::vector<glm::vec2> _jitterTable{};
    glm::vec2 _previousJitter{};

    Technique _baseTechnique{};
};

#endif
