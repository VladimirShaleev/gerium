#include "GBufferPass.hpp"

// This method handles rendering of the GBuffer pass, utilizing indirect draw calls for geometry rendering.
// It also provides a fallback path for cases where certain GPU features (like bindless textures or indirect draw count)
// are not supported.
void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    // Retrieve scene data, cluster data, and instances data from the render service.
    const auto sceneData     = renderService().scene();          // Descriptor set for camera data
    const auto clusterData   = renderService().cluster();        // Descriptor set for cluster data
    const auto instancesData = renderService().instances();      // Descriptor set for instances data
    const auto& techniques   = renderService().techniques();     // List of rendering techniques used in the scene
    auto drawCount           = renderService().instancesCount(); // Total number of instances in the scene
    auto needDestroyBuffer   = false;

    // If there are no instances to render, exit early.
    if (drawCount == 0) {
        return;
    }

    // Handles for GPU buffers storing draw commands and draw counts.
    gerium_buffer_h commands{ UndefinedHandle };
    gerium_buffer_h commandCounts{ UndefinedHandle };
    CompatCommands compatCommands{}; // Fallback structure for CPU-side culling and command generation

    // Check if the GPU supports bindless textures and indirect draw count.
    if (renderService().bindlessSupported() && renderService().drawIndirectCountSupported()) {
        // If supported, retrieve the GPU buffers for draw commands and draw counts.
        // These buffers are populated by a compute shader that performs GPU frustum culling and LOD selection.
        // The GPU generates the draw commands and counts visible meshes, reducing CPU overhead.
        check(gerium_renderer_get_buffer(renderer, "commands", &commands));
        check(gerium_renderer_get_buffer(renderer, "command_counts", &commandCounts));
    } else {
        // If indirect rendering is not supported, perform frustum culling on the CPU.
        compatCommands = compatCullingInstances();
        if (compatCommands.drawCommands.empty()) {
            return; // Skip rendering if no meshes are visible.
        }

        // Upload the CPU-generated draw commands to a GPU buffer.
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
        needDestroyBuffer = true;

        // Note: commandCounts remains UndefinedHandle because the CPU has already counted the visible meshes.
    }

    // Iterate over each rendering technique in the scene.
    for (gerium_uint32_t i = 0; i < techniques.size(); ++i) {
        const auto& technique = techniques[i];

        // Calculate offsets for the current technique in the command and count buffers.
        const auto countOffset    = i * 4;
        const auto commandsOffset = i * MAX_INSTANCES_PER_TECHNIQUE * sizeof(IndirectDraw);

        // If indirect draw count is not supported, use the CPU-computed draw count.
        if (!renderService().drawIndirectCountSupported()) {
            drawCount = compatCommands.drawCounts[i];
            if (drawCount == 0) {
                continue; // Skip if no instances are visible for this technique.
            }
        }

        // Bind the current technique and descriptor sets.
        // These calls are optimized to avoid redundant bindings.
        gerium_command_buffer_bind_technique(commandBuffer, technique);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, sceneData, SCENE_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, clusterData, CLUSTER_DATA_SET);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, instancesData, INSTANCES_DATA_SET);
        if (renderService().bindlessSupported()) {
            gerium_command_buffer_bind_descriptor_set(commandBuffer, renderService().textures(), TEXTURES_SET);
        }

        // If bindless textures and indirect draw are supported, issue a single indirect draw call.
        if (renderService().bindlessSupported() && renderService().drawIndirectSupported()) {
            gerium_command_buffer_draw_indirect(
                commandBuffer, commands, commandsOffset, commandCounts, countOffset, drawCount, sizeof(IndirectDraw));
        } else {
            // Otherwise, issue individual draw calls for each mesh.
            gerium_uint64_t lastTexturesHash = 0;

            // Iterate over the draw commands for the current technique.
            drawCount = compatCommands.drawCounts[i];
            for (gerium_uint32_t commandIndex = 0; commandIndex < drawCount; ++commandIndex) {
                const auto& command = compatCommands.drawCommands[i * MAX_INSTANCES_PER_TECHNIQUE + commandIndex];

                // If bindless textures are not supported, bind textures manually.
                if (!renderService().bindlessSupported()) {
                    compatBindTextures(renderer, commandBuffer, command.firstInstance, lastTexturesHash);
                }

                // Issue the draw call for the current mesh.
                gerium_command_buffer_draw(commandBuffer,
                                           command.firstVertex,
                                           command.vertexCount,
                                           command.firstInstance,
                                           command.instanceCount);
            }
        }
    }

    // Clean up the temporary GPU buffer if it was created for CPU-side culling.
    if (needDestroyBuffer) {
        gerium_renderer_destroy_buffer(renderer, commands);
    }
}

// Performs CPU-side frustum culling and generates draw commands for visible instances.
// This is used as a fallback when GPU culling is not supported.
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
            uint lodCount = meshes[meshIndex].lodCount;
            uint lodIndex = 0;

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
            draws[commandIndex].vertexCount   = meshes[meshIndex].lods[lodIndex].indexCount;
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

// Binds textures manually for a specific instance when bindless textures are not supported.
// This is used as a fallback for older hardware.
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
