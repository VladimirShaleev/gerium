#include "PresentPass.hpp"
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
    gerium_command_buffer_draw_profiler(commandBuffer, nullptr);

    if (ImGui::Begin("Settings")) {
        auto& settings = entityRegistry().ctx().get<Settings>();
        if (ImGui::Button("Save state")) {
            settings.state = Settings::Save;
        }
        if (ImGui::Button("Load state")) {
            settings.state = Settings::Load;
        }
    }
    ImGui::End();
}
