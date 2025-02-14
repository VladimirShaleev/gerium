#include "CullingPass.hpp"

void CullingPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    renderService().mergeStaticAndDynamicInstances(commandBuffer);

    gerium_buffer_h commandCounts;
    check(gerium_renderer_get_buffer(renderer, "command_counts", &commandCounts));

    gerium_command_buffer_bind_technique(commandBuffer, renderService().baseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().sceneData(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().clusterData(), CLUSTER_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().instancesData(), INSTANCES_DATA_SET);
    gerium_command_buffer_fill_buffer(commandBuffer, commandCounts, 0, MAX_TECHNIQUES * 4, 0);
    gerium_command_buffer_dispatch(commandBuffer, getGroupCount(renderService().instancesCount(), 64U), 1, 1);
}
