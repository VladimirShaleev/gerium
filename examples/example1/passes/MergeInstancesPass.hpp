#ifndef MERGE_INSTANCES_PASS_HPP
#define MERGE_INSTANCES_PASS_HPP

#include "RenderPass.hpp"

class MergeInstancesPass final : public RenderPass {
public:
    MergeInstancesPass() : RenderPass("merge_instances_pass") {
    }

    void render(gerium_frame_graph_t frameGraph,
                gerium_renderer_t renderer,
                gerium_command_buffer_t commandBuffer,
                gerium_uint32_t worker,
                gerium_uint32_t totalWorkers) override;
};

#endif
