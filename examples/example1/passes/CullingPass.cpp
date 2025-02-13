#include "CullingPass.hpp"

void CullingPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    gerium_buffer_h commandCounts;
    check(gerium_renderer_get_buffer(renderer, "command_counts", &commandCounts));

    auto ds = resourceManager().createDescriptorSet("");
    gerium_renderer_bind_buffer(renderer, ds, 0, renderService().instancesBuffer());
    gerium_renderer_bind_resource(renderer, ds, 1, "command_counts", false);
    gerium_renderer_bind_resource(renderer, ds, 2, "commands", false);
    gerium_renderer_bind_buffer(renderer, ds, 3, renderService().instanceCountBuffer());

    gerium_command_buffer_bind_technique(commandBuffer, renderService().baseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().sceneDataDescriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().clusterDescriptorSet(), CLUSTER_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, INSTANCES_DATA_SET);
    gerium_command_buffer_fill_buffer(commandBuffer, commandCounts, 0, MAX_TECHNIQUES * 4, 0);
    gerium_command_buffer_dispatch(commandBuffer, getGroupCount(renderService().instanceCount(), 64U), 1, 1);
}
