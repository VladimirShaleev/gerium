#ifndef GERIUM_FRAME_GRAPH_HPP
#define GERIUM_FRAME_GRAPH_HPP

#include "Handles.hpp"
#include "ObjectPtr.hpp"
#include "Renderer.hpp"

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
    gerium_render_pass_t pass;
    gerium_utf8_t name;
    gerium_data_t* data;
};

struct FrameGraphNode {
    RenderPassHandle renderPass;
    FramebufferHandle framebuffer;
    FrameGraphRenderPassHandle pass;
    gerium_utf8_t name;
    gerium_uint8_t inputCount;
    gerium_uint8_t outputCount;
    gerium_uint8_t edgeCount;
    gerium_uint8_t enabled;
    std::array<FrameGraphResourceHandle, kMaxInputs> inputs;
    std::array<FrameGraphResourceHandle, kMaxOutputs> outputs;
    std::array<FrameGraphResourceHandle, kMaxOutputs> edges;
};

struct FrameGraphResourceInfo {
    gerium_resource_type_t type;

    union {
        struct {
        } buffer;

        struct {
            gerium_format_t format;
            gerium_uint16_t width;
            gerium_uint16_t height;
            gerium_uint16_t depth;
            gerium_float32_t autoScale;
            gerium_render_pass_op_t operation;
            gerium_color_component_flags_t colorWriteMask;
            gerium_color_blend_attachment_state_t colorBlend;
            TextureHandle handle;
        } texture;
    };
};

struct FrameGraphResource {
    gerium_utf8_t name;
    gerium_bool_t external;
    gerium_uint32_t refCount;
    FrameGraphNodeHandle producer;
    FrameGraphNodeHandle output;
    FrameGraphResourceInfo info;
};

class FrameGraph : public _gerium_frame_graph {
public:
    ~FrameGraph() override;
    FrameGraph(Renderer* renderer);

    void addPass(gerium_utf8_t name, const gerium_render_pass_t* renderPass, gerium_data_t* data);
    void removePass(gerium_utf8_t name);

    void addNode(gerium_utf8_t name,
                 gerium_uint32_t inputCount,
                 const gerium_resource_input_t* inputs,
                 gerium_uint32_t outputCount,
                 const gerium_resource_output_t* outputs);

    void clear();
    void compile();
    void resize(gerium_uint16_t oldWidth,
                gerium_uint16_t newWidth,
                gerium_uint16_t oldHeight,
                gerium_uint16_t newHeight);

    const FrameGraphResource* getResource(FrameGraphResourceHandle handle) const noexcept;
    const FrameGraphResource* getResource(gerium_utf8_t name) const noexcept;

    gerium_uint32_t nodeCount() const noexcept;
    const FrameGraphNode* getNode(gerium_uint32_t index) const noexcept;
    const FrameGraphNode* getNode(gerium_utf8_t name) const noexcept;
    const FrameGraphRenderPass* getPass(FrameGraphRenderPassHandle handle) const noexcept;

private:
    using NodeHashMap       = absl::flat_hash_map<gerium_uint64_t, FrameGraphNodeHandle>;
    using ResourceHashMap   = absl::flat_hash_map<gerium_uint64_t, FrameGraphResourceHandle>;
    using RenderPassHashMap = absl::flat_hash_map<gerium_uint64_t, FrameGraphRenderPassHandle>;

    FrameGraphResourceHandle createNodeOutput(const gerium_resource_output_t& output, FrameGraphNodeHandle producer);
    FrameGraphResourceHandle createNodeInput(const gerium_resource_input_t& input);

    void computeEdges(FrameGraphNode* node);

    void calcFramebufferSize(FrameGraphResourceInfo& info) const noexcept;

    Renderer* _renderer;

    FrameGraphNodePool _nodes;
    FrameGraphResourcePool _resources;
    FrameGraphRenderPassPool _renderPasses;

    NodeHashMap _nodeCache;
    ResourceHashMap _resourceCache;
    RenderPassHashMap _renderPassCache;

    gerium_uint32_t _nodeGraphCount;
    std::array<FrameGraphNodeHandle, kMaxNodes> _nodeGraph;

    std::array<FrameGraphNodeHandle, kMaxNodes> _sortedNodes;
    std::array<FrameGraphNodeHandle, kMaxNodes> _stack;
    std::array<uint8_t, kMaxNodes> _visited;
    std::array<TextureHandle, kMaxOutputs> _freeList;
    std::array<FrameGraphNodeHandle, kMaxNodes> _allocations;
};

} // namespace gerium

#endif
