#include "GBufferPass.hpp"

// This method demonstrates modern rendering with indirect draw call for all geometry,
// as well as a workaround if a particular feature is not supported.
void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    const auto sceneData     = renderService().sceneData();      // descriptor set of camera
    const auto clusterData   = renderService().clusterData();    // descriptor set of claster
    const auto instancesData = renderService().instancesData();  // descriptor set of instances
    const auto& techniques   = renderService().techniques();     // vector of techniques using in the scene
    auto drawCount           = renderService().instancesCount(); // total instances in scene

    // If the scene is empty, you don't need to make any rendering calls.
    if (drawCount == 0) {
        return;
    }

    gerium_buffer_h commands{ UndefinedHandle };
    gerium_buffer_h commandCounts{ UndefinedHandle };
    CompatCommands compatCommands{};

    // If the device supports bindless textures and indirect rendering with draw count SSBO,
    // then we get a SSBO with draw commands and a SSBO with draw count.
    if (renderService().bindlessSupported() && renderService().drawIndirectCountSupported()) {
        // These SSBO buffers are filled with pass in the compute shader. This compute shader
        // produces GPU frustum culling, as well as the selection of the mesh lod. The resulting
        // commands buffer will contain the rendering commands with the mesh lod index. And in
        // command_counts, the number of visible meshes on the screen.
        //
        // This is a fairly efficient rendering, because we send a command buffer to the GPU in one call.
        // And already the GPU itself will form the drawing commands for the visible geometry, after which
        // it will begin to draw them. Apart from updating the camera, the CPU may not be used at all for
        // scene rendering, much less data synchronization.
        check(gerium_renderer_get_buffer(renderer, "commands", &commands));
        check(gerium_renderer_get_buffer(renderer, "command_counts", &commandCounts));
    } else {
        // If we cannot use indirect rendering, we need to perform frustum culling on the CPU side.
        compatCommands = compatCullingInstances();
        if (compatCommands.drawCommands.empty()) {
            return; // if no one mesh is visible (no drawing commands), then we do not do any drawings
        }
        // Fill the dynamic SSBO with drawing commands
        auto size = compatCommands.drawCommands.size() * sizeof(IndirectDraw);
        gerium_buffer_h buffer;
        check(gerium_renderer_create_buffer(renderer,
                                            GERIUM_BUFFER_USAGE_STORAGE_BIT | GERIUM_BUFFER_USAGE_INDIRECT_BIT,
                                            true,
                                            "",
                                            compatCommands.drawCommands.data(),
                                            size,
                                            &commands));
        gerium_renderer_bind_buffer(renderer, instancesData, 4, commands);

        // It is also worth noting that commandCounts remains UndefinedHandle, because we have already counted the
        // number of draws (visible grids) for each technique on the CPU side, and if indirect rendering is supported,
        // then when calling the gerium_command_buffer_draw_indirect function with draw_count_handle equal to
        // UndefinedHandle, the draw_count argument will be used as the number of draws for indirect rendering.
        // Otherwise, the number of draws is taken from draw_count_handle, and draw_count indicates the maximum
        // possible number of draws. And as you can see, this branching is exactly what is needed for the case
        // when we cannot use draw_count_handle.
    }

    // Need to go through each material in the scene
    for (gerium_uint32_t i = 0; i < techniques.size(); ++i) {
        const auto& technique = techniques[i];
        // Offsets in command_counts and commands for current technique
        const auto countOffset    = i * 4;
        const auto commandsOffset = i * MAX_INSTANCES_PER_TECHNIQUE * sizeof(IndirectDraw);
        if (!renderService().drawIndirectCountSupported()) { // if draw indirect is not supported
            drawCount = compatCommands.drawCounts[i];
            if (drawCount == 0) {
                continue; // if there are no renderings for the current technique, we move on to the next technique
            }
        }
        // Bind current technique and descriptor sets
        // (if the descriptor set or technique is already bind, it will not be bind again)
        gerium_command_buffer_bind_technique(commandBuffer, technique);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, sceneData, SCENE_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, clusterData, CLUSTER_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, instancesData, INSTANCES_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().textures(), TEXTURES_SET);

        // If bindless textures is supported and indirect draw is supported
        if (renderService().bindlessSupported() && renderService().drawIndirectSupported()) {
            // We draw all the geometry for this technique in one call
            gerium_command_buffer_draw_indirect(
                commandBuffer, commands, commandsOffset, commandCounts, countOffset, drawCount, sizeof(IndirectDraw));
        } else {
            // Otherwise we'll have to make a separate draw call for each mesh

            gerium_uint64_t lastTexturesHash = 0;

            // Enumerate the draw commands for the current technique
            drawCount = compatCommands.drawCounts[i];
            for (gerium_uint32_t commandIndex = 0; commandIndex < drawCount; ++commandIndex) {
                const auto& command = compatCommands.drawCommands[i * MAX_INSTANCES_PER_TECHNIQUE + commandIndex];

                // If bindless textures not supported
                if (!renderService().bindlessSupported()) {
                    compatBindTextures(renderer, commandBuffer, command.firstInstance, lastTexturesHash);
                }

                // Draw call
                gerium_command_buffer_draw(commandBuffer,
                                           command.firstVertex,
                                           command.vertexCount,
                                           command.firstInstance,
                                           command.instanceCount);
            }
        }
    }

    // If we filled the command buffer from the CPU side in this frame, we will queue it for deletion
    if (!renderService().bindlessSupported() || !renderService().drawIndirectCountSupported()) {
        gerium_renderer_destroy_buffer(renderer, commands);
    }
}

// Frustum culling on the CPU side (for devices with legacy pipeline)
GBufferPass::CompatCommands GBufferPass::compatCullingInstances() {
    const auto& scene     = renderService().compatSceneData();
    const auto& meshes    = renderService().compatMeshes();
    const auto& instances = renderService().compatInstances();

    std::vector<IndirectDraw> draws;
    std::vector<gerium_uint32_t> drawCounts;
    drawCounts.resize(renderService().techniques().size());
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

// Bind textures (for devices with legacy pipeline)
void GBufferPass::compatBindTextures(gerium_renderer_t renderer,
                                     gerium_command_buffer_t commandBuffer,
                                     gerium_uint32_t instance,
                                     gerium_uint64_t& lastTexturesHash) {
    // Get material index by instance index
    const auto materialIndex = renderService().compatInstances()[instance].material;

    // Get material by material index
    const auto& material = renderService().compatMaterials()[materialIndex];

    auto seed = RAPID_SEED;
    seed      = rapidhash_withSeed(&material.baseColorTexture, sizeof(glm::uint), seed);
    seed      = rapidhash_withSeed(&material.metallicRoughnessTexture, sizeof(glm::uint), seed);
    seed      = rapidhash_withSeed(&material.normalTexture, sizeof(glm::uint), seed);
    seed      = rapidhash_withSeed(&material.occlusionTexture, sizeof(glm::uint), seed);
    seed      = rapidhash_withSeed(&material.emissiveTexture, sizeof(glm::uint), seed);
    if (seed == lastTexturesHash) {
        return; // if the texture for the material not has changed, return
    }
    auto textureSet = renderService().resourceManager().createDescriptorSet("");
    gerium_renderer_bind_texture(renderer, textureSet, 0, 0, { gerium_uint16_t(material.baseColorTexture) });
    gerium_renderer_bind_texture(renderer, textureSet, 1, 0, { gerium_uint16_t(material.metallicRoughnessTexture) });
    gerium_renderer_bind_texture(renderer, textureSet, 2, 0, { gerium_uint16_t(material.normalTexture) });
    gerium_renderer_bind_texture(renderer, textureSet, 3, 0, { gerium_uint16_t(material.occlusionTexture) });
    gerium_renderer_bind_texture(renderer, textureSet, 4, 0, { gerium_uint16_t(material.emissiveTexture) });
    gerium_command_buffer_bind_descriptor_set(commandBuffer, textureSet, TEXTURES_SET);
    lastTexturesHash = seed;
}
