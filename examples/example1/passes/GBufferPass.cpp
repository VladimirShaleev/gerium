#include "GBufferPass.hpp"

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    // const auto sceneData = renderService().getActiveSceneDescriptorSet();

    // for (const auto& mesh : entityRegistry().getAllComponents<Mesh>()) {
    //     const auto entity = entityRegistry().getEntityFromComponent(mesh);
    //     const auto model  = resourceManager().loadModel(mesh.modelName);
    //     for (auto meshIndex : mesh.meshIndices) {
    //         const auto& data      = model->meshes()[meshIndex];
    //         const auto& transform = entityRegistry().getComponent<Transform>(entity);

    //         auto ds = resourceManager().createDescriptorSet("");
    //         auto b =
    //             resourceManager().createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "", nullptr,
    //             sizeof(MeshDataT));

    //         auto d          = (MeshDataT*) gerium_renderer_map_buffer(renderer, b, 0, sizeof(MeshDataT));
    //         d->world        = (glm::mat4&) transform.world;
    //         d->prevWorld    = (glm::mat4&) transform.prevWorld;
    //         d->inverseWorld = glm::inverse((glm::mat4&) transform.world);

    //         gerium_renderer_unmap_buffer(renderer, b);
    //         gerium_renderer_bind_buffer(renderer, ds, 0, b);

    //         gerium_command_buffer_bind_technique(commandBuffer, renderService().baseTechnique());
    //         gerium_command_buffer_bind_descriptor_set(commandBuffer, sceneData, SCENE_DATA_SET);
    //         gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, MESH_DATA_SET);
    //         gerium_command_buffer_bind_vertex_buffer(commandBuffer, data.positions, 0, data.positionsOffset);
    //         gerium_command_buffer_bind_vertex_buffer(commandBuffer, data.texcoords, 1, data.texcoordsOffset);
    //         gerium_command_buffer_bind_vertex_buffer(commandBuffer, data.normals, 2, data.normalsOffset);
    //         gerium_command_buffer_bind_vertex_buffer(commandBuffer, data.tangents, 3, data.tangentsOffset);
    //         gerium_command_buffer_bind_index_buffer(commandBuffer, data.indices, data.indicesOffset, data.indexType);
    //         gerium_command_buffer_draw_indexed(commandBuffer, 0, data.primitiveCount, 0, 0, 1);
    //     }
    // }

    gerium_buffer_h commandCount;
    gerium_buffer_h commands;
    check(gerium_renderer_get_buffer(renderer, "command_count", &commandCount));
    check(gerium_renderer_get_buffer(renderer, "commands", &commands));

    auto ds = resourceManager().createDescriptorSet("");
    gerium_renderer_bind_buffer(renderer, ds, 0, renderService().instancesBuffer());
    gerium_renderer_bind_resource(renderer, ds, 1, "commands", false);

    const auto count = renderService().instanceCount();

    gerium_command_buffer_bind_technique(commandBuffer, renderService().baseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().sceneDataDescriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().clusterDescriptorSet(), CLUSTER_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, INSTANCES_DATA_SET);
    gerium_command_buffer_draw_indirect(commandBuffer, commands, 0, commandCount, 0, count, sizeof(IndirectDraw));
}
