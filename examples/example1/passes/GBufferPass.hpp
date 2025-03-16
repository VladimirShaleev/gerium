#ifndef GBUFFER_PASS_HPP
#define GBUFFER_PASS_HPP

#include "RenderPass.hpp"

// GBufferPass is a rendering pass responsible for generating the G-Buffer,
// which typically stores geometry information like positions, normals, and materials.
// This pass supports both modern GPU features (e.g., indirect draw) and fallback paths for older hardware.
class GBufferPass final : public RenderPass {
public:
    GBufferPass() : RenderPass("gbuffer_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;

private:
    // A structure to hold fallback rendering commands when GPU features like indirect draw are not supported.
    // This is used for CPU-side frustum culling and command generation.
    struct CompatCommands {
        std::vector<IndirectDraw> drawCommands;  // List of indirect draw commands for visible meshes
        std::vector<gerium_uint32_t> drawCounts; // Number of draw commands per rendering technique
    };

    // Performs CPU-side frustum culling and generates draw commands for visible instances.
    // This is used as a fallback when GPU culling is not supported.
    CompatCommands compatCullingInstances();

    // Binds textures manually for a specific instance when bindless textures are not supported.
    // This is used as a fallback for older hardware.
    void compatBindTextures(gerium_renderer_t renderer,
                            gerium_command_buffer_t commandBuffer,
                            gerium_uint32_t instance,
                            gerium_uint64_t& lastTexturesHash);
};

#endif
