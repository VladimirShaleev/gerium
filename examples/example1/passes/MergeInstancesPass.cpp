#include "MergeInstancesPass.hpp"

void MergeInstancesPass::render(gerium_frame_graph_t frameGraph,
                                gerium_renderer_t renderer,
                                gerium_command_buffer_t commandBuffer,
                                gerium_uint32_t worker,
                                gerium_uint32_t totalWorkers) {
    renderService().mergeStaticAndDynamicInstances(commandBuffer);
}
