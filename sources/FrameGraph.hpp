#ifndef GERIUM_FRAME_GRAPH_HPP
#define GERIUM_FRAME_GRAPH_HPP

#include "Handles.hpp"
#include "Logger.hpp"
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
    gerium_data_t data;
};

struct FrameGraphNode {
    RenderPassHandle renderPass;
    FramebufferHandle framebuffers[2];
    FrameGraphRenderPassHandle pass;
    gerium_utf8_t name;
    gerium_uint8_t compute;
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
            gerium_uint32_t size;
            gerium_uint32_t fillValue;
            gerium_buffer_usage_flags_t usage;
            BufferHandle handle;
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
            gerium_clear_color_attachment_state_t clearColor;
            gerium_clear_depth_stencil_attachment_state_t clearDepthStencil;
            TextureHandle handles[2];
        } texture;
    };
};

struct FrameGraphResource {
    gerium_utf8_t name;
    gerium_bool_t external;
    gerium_bool_t saveForNextFrame;
    gerium_uint32_t refCount;
    FrameGraphNodeHandle producer;
    FrameGraphNodeHandle output;
    FrameGraphResourceInfo info;
};

struct FrameGraphExternalResource {
    Handle handle;
    bool texture;
};

class FrameGraph : public _gerium_frame_graph {
public:
    ~FrameGraph() override;
    FrameGraph(Renderer* renderer);

    void addPass(gerium_utf8_t name, const gerium_render_pass_t* renderPass, gerium_data_t data);
    void removePass(gerium_utf8_t name);

    void addNode(gerium_utf8_t name,
                 bool compute,
                 gerium_uint32_t inputCount,
                 const gerium_resource_input_t* inputs,
                 gerium_uint32_t outputCount,
                 const gerium_resource_output_t* outputs);
    void addBuffer(gerium_utf8_t name, BufferHandle handle);
    void addTexture(gerium_utf8_t name, TextureHandle handle);

    void clear();
    void compile();
    void resize(gerium_uint16_t oldWidth,
                gerium_uint16_t newWidth,
                gerium_uint16_t oldHeight,
                gerium_uint16_t newHeight);

    const FrameGraphResource* getResource(FrameGraphResourceHandle handle) const noexcept;
    const FrameGraphResource* getResource(gerium_utf8_t name) const noexcept;

    void fillExternalResource(FrameGraphResourceHandle handle) noexcept;

    gerium_uint32_t nodeCount() const noexcept;
    const FrameGraphNode* getNode(gerium_uint32_t index) const noexcept;
    const FrameGraphNode* getNode(gerium_utf8_t name) const noexcept;
    const FrameGraphRenderPass* getPass(FrameGraphRenderPassHandle handle) const noexcept;

private:
    using NodeHashMap       = absl::flat_hash_map<gerium_uint64_t, FrameGraphNodeHandle>;
    using ResourceHashMap   = absl::flat_hash_map<gerium_uint64_t, FrameGraphResourceHandle>;
    using RenderPassHashMap = absl::flat_hash_map<gerium_uint64_t, FrameGraphRenderPassHandle>;
    using ExternalHashMap   = absl::flat_hash_map<gerium_uint64_t, FrameGraphExternalResource>;

    FrameGraphResourceHandle createNodeOutput(const gerium_resource_output_t& output, FrameGraphNodeHandle producer);
    FrameGraphResourceHandle createNodeInput(const gerium_resource_input_t& input);

    void computeEdges(FrameGraphNode* node);

    void calcFramebufferSize(FrameGraphResourceInfo& info) const noexcept;

    ObjectPtr<Logger> _logger;
    Renderer* _renderer;
    bool _hasChanges;

    FrameGraphNodePool _nodes;
    FrameGraphResourcePool _resources;
    FrameGraphRenderPassPool _renderPasses;

    NodeHashMap _nodeCache;
    ResourceHashMap _resourceCache;
    RenderPassHashMap _renderPassCache;
    ExternalHashMap _externalCache;

    gerium_uint32_t _nodeGraphCount;
    std::array<FrameGraphNodeHandle, kMaxNodes> _nodeGraph;

    std::array<FrameGraphNodeHandle, kMaxNodes> _sortedNodes;
    std::array<FrameGraphNodeHandle, kMaxNodes> _stack;
    std::array<uint8_t, kMaxNodes> _visited;
    std::array<FrameGraphNodeHandle, kMaxNodes> _allocations;
    std::set<TextureHandle> _freeList;
};

} // namespace gerium

#endif
