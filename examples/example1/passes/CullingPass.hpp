#ifndef CULLING_PASS_HPP
#define CULLING_PASS_HPP

#include "RenderPass.hpp"

// CullingPass is a rendering pass responsible for performing frustum culling and LOD selection.
// It uses a compute shader to determine which instances are visible and generates indirect draw commands
// for the visible geometry.
class CullingPass final : public RenderPass {
public:
    CullingPass() : RenderPass("culling_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
};

#endif
