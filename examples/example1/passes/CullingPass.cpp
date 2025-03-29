#include "CullingPass.hpp"

// CullingPass is responsible for performing frustum culling and LOD selection on the GPU.
// This pass prepares indirect draw commands and counts visible instances for each rendering technique.
void CullingPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    // Merge static and dynamic instances into a single buffer for culling.
    // This ensures that all instances are processed together during the culling pass.
    renderService().mergeStaticAndDynamicInstances(commandBuffer);

    // Retrieve the buffer that will store the draw counts for each rendering technique.
    // This buffer is populated by the compute shader during the culling pass.
    gerium_buffer_h commandCounts;
    check(gerium_renderer_get_buffer(renderer, "command_counts", &commandCounts));

    // Bind the culling technique (compute shader) and required descriptor sets.
    // These sets provide access to scene data, cluster data, and instances data.
    gerium_command_buffer_bind_technique(commandBuffer, renderService().resourceManager().loadTechnique(TECH_BASE_ID));
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().scene(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().cluster(), CLUSTER_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().instances(), INSTANCES_DATA_SET);

    // Clear the command counts buffer to zero before starting the culling process.
    // This ensures that the buffer is in a known state before the compute shader writes to it.
    gerium_command_buffer_fill_buffer(commandBuffer, commandCounts, 0, MAX_TECHNIQUES * 4, 0);

    // Dispatch the compute shader to perform frustum culling and LOD selection.
    // The number of workgroups is calculated based on the total number of instances.
    gerium_command_buffer_dispatch(commandBuffer, getGroupCount(renderService().instancesCount(), 64U), 1, 1);
}
