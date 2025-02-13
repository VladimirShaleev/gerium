#include "GBufferPass.hpp"

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    gerium_buffer_h commandCounts;
    gerium_buffer_h commands;
    check(gerium_renderer_get_buffer(renderer, "command_counts", &commandCounts));
    check(gerium_renderer_get_buffer(renderer, "commands", &commands));

    auto ds = resourceManager().createDescriptorSet("");
    gerium_renderer_bind_buffer(renderer, ds, 0, renderService().instancesBuffer());
    gerium_renderer_bind_resource(renderer, ds, 1, "commands", false);
    gerium_renderer_bind_buffer(renderer, ds, 2, renderService().materialsBuffer());

    const auto count     = renderService().instanceCount();
    const auto sceneData = renderService().sceneDataDescriptorSet();
    const auto cluster   = renderService().clusterDescriptorSet();

    for (gerium_uint32_t i = 0; i < renderService().staticTechniques().size(); ++i) {
        const auto technique      = renderService().staticTechniques()[i];
        const auto countOffset    = i * 4;
        const auto commandsOffset = i * MAX_INSTANCES_PER_TECHNIQUE * sizeof(IndirectDraw);
        gerium_command_buffer_bind_technique(commandBuffer, technique);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, sceneData, SCENE_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, cluster, CLUSTER_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, INSTANCES_DATA_SET);
        gerium_command_buffer_draw_indirect(
            commandBuffer, commands, commandsOffset, commandCounts, countOffset, count, sizeof(IndirectDraw));
    }
}
