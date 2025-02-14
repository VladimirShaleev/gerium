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

    const auto sceneData      = renderService().sceneData();
    const auto clusterData    = renderService().clusterData();
    const auto instancesData  = renderService().instancesData();
    const auto instancesCount = renderService().instancesCount();
    const auto textures       = renderService().textures();

    const auto& techniques = renderService().techniques();

    for (gerium_uint32_t i = 0; i < techniques.size(); ++i) {
        const auto& technique     = techniques[i];
        const auto countOffset    = i * 4;
        const auto commandsOffset = i * MAX_INSTANCES_PER_TECHNIQUE * sizeof(IndirectDraw);
        gerium_command_buffer_bind_technique(commandBuffer, technique);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, sceneData, SCENE_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, clusterData, CLUSTER_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, instancesData, INSTANCES_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, textures, TEXTURES_SET);
        gerium_command_buffer_draw_indirect(
            commandBuffer, commands, commandsOffset, commandCounts, countOffset, instancesCount, sizeof(IndirectDraw));
    }
}
