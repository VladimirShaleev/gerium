#include "GBufferPass.hpp"

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    const auto sceneData     = renderService().sceneData();
    const auto clusterData   = renderService().clusterData();
    const auto instancesData = renderService().instancesData();
    const auto textures      = renderService().textures();
    const auto& techniques   = renderService().techniques();
    auto drawCount           = renderService().instancesCount();

    if (drawCount == 0) {
        return;
    }

    gerium_buffer_h commands{ UndefinedHandle };
    gerium_buffer_h commandCounts{ UndefinedHandle };
    CompatCommands compatCommands{};
    if (renderService().drawIndirectCountSupported()) {
        check(gerium_renderer_get_buffer(renderer, "commands", &commands));
        check(gerium_renderer_get_buffer(renderer, "command_counts", &commandCounts));
    } else {
        compatCommands = compatCullingInstances();
        compatCullingBuffer(renderer, compatCommands, &commands);
        gerium_renderer_bind_buffer(renderer, instancesData, 4, commands);
    }

    for (gerium_uint32_t i = 0; i < techniques.size(); ++i) {
        const auto& technique     = techniques[i];
        const auto countOffset    = i * 4;
        const auto commandsOffset = i * MAX_INSTANCES_PER_TECHNIQUE * sizeof(IndirectDraw);
        if (!renderService().drawIndirectCountSupported()) {
            drawCount = compatCommands.drawCounts[i];
            if (drawCount == 0) {
                continue;
            }
        }
        gerium_command_buffer_bind_technique(commandBuffer, technique);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, sceneData, SCENE_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, clusterData, CLUSTER_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, instancesData, INSTANCES_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, textures, TEXTURES_SET);
        if (renderService().drawIndirectSupported()) {
            gerium_command_buffer_draw_indirect(
                commandBuffer, commands, commandsOffset, commandCounts, countOffset, drawCount, sizeof(IndirectDraw));
        } else {
            drawCount = compatCommands.drawCounts[i];
            for (gerium_uint32_t commandIndex = 0; commandIndex < drawCount; ++commandIndex) {
                const auto& command = compatCommands.drawCommands[i * MAX_INSTANCES_PER_TECHNIQUE + commandIndex];
                gerium_command_buffer_draw(commandBuffer,
                                           command.firstVertex,
                                           command.vertexCount,
                                           command.firstInstance,
                                           command.instanceCount);
            }
        }
    }

    if (!renderService().drawIndirectCountSupported()) {
        gerium_renderer_destroy_buffer(renderer, commands);
    }
}

GBufferPass::CompatCommands GBufferPass::compatCullingInstances() {
    const auto& scene     = renderService().compatSceneData();
    const auto& meshes    = renderService().compatMeshes();
    const auto& instances = renderService().compatInstances();

    std::vector<IndirectDraw> draws;
    std::vector<gerium_uint32_t> drawCounts;
    for (gerium_uint32_t index = 0; index < instances.size(); ++index) {
        const auto& instance = instances[index];
        const auto meshIndex = instance.mesh;

        auto center =
            (scene.view * instance.world *
             glm::vec4(meshes[meshIndex].center[0], meshes[meshIndex].center[1], meshes[meshIndex].center[2], 1.0f))
                .xyz();
        auto radius = instance.scale * meshes[meshIndex].radius;

        auto visible = true;
        visible      = visible && center.z * scene.frustum.y - glm::abs(center.x) * scene.frustum.x > -radius;
        visible      = visible && center.z * scene.frustum.w - glm::abs(center.y) * scene.frustum.z > -radius;
        visible      = visible && center.z + radius > scene.farNear.y && center.z - radius < scene.farNear.x;

        if (visible) {
            glm::uint lodCount = meshes[meshIndex].lodCount;
            glm::uint lodIndex = 0;

            auto distance  = glm::max(glm::length(center) - radius, 0.0f);
            auto threshold = distance * scene.lodTarget / instance.scale;

            for (uint i = 1; i < lodCount; ++i) {
                if (meshes[meshIndex].lods[i].lodError < threshold) {
                    lodIndex = i;
                }
            }

            if (drawCounts.size() <= instance.technique) {
                drawCounts.resize(instance.technique + 1);
            }

            uint commandIndex = instance.technique * MAX_INSTANCES_PER_TECHNIQUE + drawCounts[instance.technique]++;

            if (draws.size() <= commandIndex || draws.size() <= index) {
                draws.resize(std::max(commandIndex, index) + 1);
            }
            draws[commandIndex].vertexCount   = meshes[meshIndex].lods[lodIndex].primitiveCount;
            draws[commandIndex].instanceCount = 1;
            draws[commandIndex].firstVertex   = 0;
            draws[commandIndex].firstInstance = index;

            // The LOD index is stored in the command buffer not by `commandIndex`, but by `index`.
            // This is necessary so that the vertex shader can access the command data by
            // `gl_InstanceIndex`. This will work because the instance index is unique within
            // a pass. In reality, it would be more correct to store the LOD by `commandIndex`,
            // and then get the command used for rendering in the vertex shader using `gl_DrawIDARB`
            // (as `uint commandIndex = instance.technique * MAX_INSTANCES_PER_TECHNIQUE + gl_DrawIDARB`).
            // But `GL_ARB_shader_draw_parameters` is not supported everywhere.
            draws[index].lodIndex = lodIndex;
        }
    }

    return { draws, drawCounts };
}

void GBufferPass::compatCullingBuffer(gerium_renderer_t renderer,
                                      const CompatCommands& compatCommands,
                                      gerium_buffer_h* commands) {
    if (!compatCommands.drawCommands.empty()) {
        auto size = compatCommands.drawCommands.size() * sizeof(IndirectDraw);
        gerium_buffer_h buffer;
        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_STORAGE_BIT | GERIUM_BUFFER_USAGE_INDIRECT_BIT,
                                            true,
                                            "",
                                            compatCommands.drawCommands.data(),
                                            size,
                                            commands));
    }
}
