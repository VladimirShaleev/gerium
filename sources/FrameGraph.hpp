#ifndef GERIUM_FRAME_GRAPH_HPP
#define GERIUM_FRAME_GRAPH_HPP

#include "Handles.hpp"
#include "ObjectPtr.hpp"

struct _gerium_frame_graph : public gerium::Object {};

namespace gerium {

constexpr uint32_t kMaxInputs  = 16;
constexpr uint32_t kMaxOutputs = 16;
constexpr uint32_t kMaxNodes   = 256;

struct FrameGraphRenderPassHandle : Handle {};

struct FrameGraphNodeHandle : Handle {};

struct FrameGraphResourceHandle : Handle {};

using FrameGraphRenderPassPool = ResourcePool<struct FrameGraphRenderPass, FrameGraphRenderPassHandle>;
using FrameGraphNodePool       = ResourcePool<struct FrameGraphNode, FrameGraphNodeHandle>;
using FrameGraphResourcePool   = ResourcePool<struct FrameGraphResource, FrameGraphResourceHandle>;

struct FrameGraphRenderPass {
    const gerium_render_pass_t* pass;
    gerium_utf8_t name;
    gerium_data_t* data;
};

struct FrameGraphNode {
    RenderPassHandle renderPass;
    FramebufferHandle framebuffer;
    const gerium_render_pass_t* pass;
    gerium_utf8_t name;
    gerium_uint8_t inputCount;
    gerium_uint8_t outputCount;
    gerium_uint8_t edgeCount;
    gerium_uint8_t enabled;
    std::array<FrameGraphResourceHandle, kMaxInputs> inputs;
    std::array<FrameGraphResourceHandle, kMaxOutputs> outputs;
    std::array<FrameGraphResourceHandle, kMaxOutputs> edges;
};

struct FrameGraphResource {
    gerium_resource_type_t type;
    gerium_utf8_t name;
    gerium_bool_t external;
    gerium_format_t format;
    gerium_uint16_t width;
    gerium_uint16_t height;
    gerium_render_pass_operation_t operation;
    FrameGraphNodeHandle producer;
    FrameGraphNodeHandle output;
};

class FrameGraph : public _gerium_frame_graph {
public:
    FrameGraph();

    gerium_result_t addPass(gerium_utf8_t name, const gerium_render_pass_t* renderPass, gerium_data_t* data);
    gerium_result_t removePass(gerium_utf8_t name);

    gerium_result_t addNode(gerium_utf8_t name,
                            gerium_uint32_t inputCount,
                            const gerium_resource_input_t* inputs,
                            gerium_uint32_t outputCount,
                            const gerium_resource_output_t* outputs);

    gerium_result_t compile() {
        compileGraph();
        return GERIUM_RESULT_SUCCESS;
    }

private:
    using NodeHashMap       = absl::flat_hash_map<gerium_uint64_t, FrameGraphNodeHandle>;
    using ResourceHashMap   = absl::flat_hash_map<gerium_uint64_t, FrameGraphResourceHandle>;
    using RenderPassHashMap = absl::flat_hash_map<gerium_uint64_t, FrameGraphRenderPassHandle>;

    FrameGraphResourceHandle createNodeOutput(const gerium_resource_output_t& output, FrameGraphNodeHandle producer);
    FrameGraphResourceHandle createNodeInput(const gerium_resource_input_t& input);

    void compileGraph();
    void computeEdges(FrameGraphNode* node);
    void clearGraph() noexcept;

    FrameGraphResource* findResource(gerium_utf8_t name) noexcept;

    FrameGraphNodePool _nodes;
    FrameGraphResourcePool _resources;
    FrameGraphRenderPassPool _rendererPasses;

    NodeHashMap _nodeCache;
    ResourceHashMap _resourceCache;
    RenderPassHashMap _renderPassCache;

    gerium_uint32_t _nodeGraphCount;
    std::array<FrameGraphNodeHandle, kMaxNodes> _nodeGraph;
};

} // namespace gerium

#endif
