#include "MergeInstancesPass.hpp"

// MergeInstancesPass is a stub pass used when frustum culling and command generation
// are performed on the CPU instead of the GPU. Its primary purpose is to merge static
// and dynamic instances into a single buffer
void MergeInstancesPass::render(gerium_frame_graph_t frameGraph,
                                gerium_renderer_t renderer,
                                gerium_command_buffer_t commandBuffer,
                                gerium_uint32_t worker,
                                gerium_uint32_t totalWorkers) {
    // Merge static and dynamic instances into a single buffer for culling.
    renderService().mergeStaticAndDynamicInstances(commandBuffer);
}
