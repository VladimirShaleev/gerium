#include "PresentPass.hpp"
#include "../Application.hpp"
#include "../components/Settings.hpp"

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto technique = resourceManager().loadTechnique(TECH_POSTPROCESS_ID);

    auto ds = resourceManager().createDescriptorSet("");
    gerium_renderer_bind_resource(renderer, ds, 0, "base", false);

    gerium_command_buffer_bind_technique(commandBuffer, technique);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, 0);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);

    const auto& settings = renderService().settings();
    if (settings.developerMode) {
        if (!_developerUI) {
            _developerUI = std::make_unique<DeveloperUI>(entityRegistry(), renderService().application().dispatcher());
        }
        gerium_renderer_set_profiler_enable(renderer, settings.showProfiler);
        _developerUI->show(commandBuffer);
    } else {
        _developerUI = nullptr;
    }
}
