#include "RenderService.hpp"

#include "../Application.hpp"
#include "../passes/PresentPass.hpp"
#include <ranges>

gerium_technique_h RenderService::baseTechnique() const noexcept {
    return _baseTechnique;
}

void RenderService::start() {
    gerium_renderer_options_t options{};
    options.app_version       = GERIUM_VERSION_ENCODE(1, 0, 0);
    options.dynamic_ssbo_size = 64 * 1024 * 1024;

#ifdef NDEBUG
    options.debug_mode = false;
#else
    options.debug_mode = true;
#endif

    check(gerium_renderer_create(application().handle(), GERIUM_FEATURE_BINDLESS_BIT, &options, &_renderer));
    gerium_renderer_set_profiler_enable(_renderer, true);
    check(gerium_profiler_create(_renderer, &_profiler));

    _bindlessSupported = gerium_renderer_get_enabled_features(_renderer) & GERIUM_FEATURE_BINDLESS_BIT;

    check(gerium_frame_graph_create(_renderer, &_frameGraph));

    _resourceManager.create(_renderer, _frameGraph);

    addPass<PresentPass>();

    _resourceManager.loadFrameGraph("main");
    check(gerium_frame_graph_compile(_frameGraph));

    _baseTechnique = _resourceManager.loadTechnique("base");

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }
}

void RenderService::stop() {
    if (_renderer) {
        for (auto& _renderPasse : std::ranges::reverse_view(_renderPasses)) {
            _renderPasse->uninitialize(_frameGraph, _renderer);
        }

        _baseTechnique = nullptr;

        _resourceManager.destroy();

        if (_frameGraph) {
            gerium_frame_graph_destroy(_frameGraph);
            _frameGraph = nullptr;
        }
        if (_profiler) {
            gerium_profiler_destroy(_profiler);
            _profiler = nullptr;
        }
        gerium_renderer_destroy(_renderer);
        _renderer = nullptr;
    }
}

void RenderService::update() {
    if (gerium_renderer_new_frame(_renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return;
    }

    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);

    if (_error) {
        std::rethrow_exception(_error);
    }
}

gerium_uint32_t RenderService::prepare(gerium_frame_graph_t frameGraph,
                                       gerium_renderer_t renderer,
                                       gerium_uint32_t maxWorkers,
                                       gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        return renderPass->prepare(frameGraph, renderer, maxWorkers);
    } catch (...) {
        renderPass->renderService()._error = std::current_exception();
        return 0;
    }
}

gerium_bool_t RenderService::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        renderPass->resize(frameGraph, renderer);
    } catch (...) {
        renderPass->renderService()._error = std::current_exception();
        return false;
    }
    return true;
}

gerium_bool_t RenderService::render(gerium_frame_graph_t frameGraph,
                                    gerium_renderer_t renderer,
                                    gerium_command_buffer_t commandBuffer,
                                    gerium_uint32_t worker,
                                    gerium_uint32_t totalWorkers,
                                    gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        renderPass->render(frameGraph, renderer, commandBuffer, worker, totalWorkers);
    } catch (...) {
        renderPass->renderService()._error = std::current_exception();
        return false;
    }
    return true;
}
