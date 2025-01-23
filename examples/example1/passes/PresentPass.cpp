#include "PresentPass.hpp"

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    gerium_command_buffer_bind_technique(commandBuffer, renderService().baseTechnique());
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);

    if constexpr (debugBuild) {
        gerium_command_buffer_draw_profiler(commandBuffer, nullptr);
    }
}
