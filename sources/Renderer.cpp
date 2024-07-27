#include "Renderer.hpp"
#include "FrameGraph.hpp"

namespace gerium {

Renderer::Renderer() noexcept {
}

void Renderer::initialize(gerium_uint32_t version, bool debug) {
    onInitialize(version, debug);
}

bool Renderer::getProfilerEnable() const noexcept {
    return onGetProfilerEnable();
}

void Renderer::setProfilerEnable(bool enable) noexcept {
    onSetProfilerEnable(enable);
}

BufferHandle Renderer::createBuffer(const BufferCreation& creation) {
    return onCreateBuffer(creation);
}

TextureHandle Renderer::createTexture(const TextureCreation& creation) {
    return onCreateTexture(creation);
}

MaterialHandle Renderer::createMaterial(const FrameGraph& frameGraph,
                                        gerium_utf8_t name,
                                        gerium_uint32_t pipelineCount,
                                        const gerium_pipeline_t* pipelines) {
    return onCreateMaterial(frameGraph, name, pipelineCount, pipelines);
}

DescriptorSetHandle Renderer::createDescriptorSet() {
    return onCreateDescriptorSet();
}

RenderPassHandle Renderer::createRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    return onCreateRenderPass(frameGraph, node);
}

FramebufferHandle Renderer::createFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    return onCreateFramebuffer(frameGraph, node);
}

void Renderer::destroyBuffer(BufferHandle handle) noexcept {
    onDestroyBuffer(handle);
}

void Renderer::destroyTexture(TextureHandle handle) noexcept {
    onDestroyTexture(handle);
}

void Renderer::destroyMaterial(MaterialHandle handle) noexcept {
    onDestroyMaterial(handle);
}

void Renderer::destroyDescriptorSet(DescriptorSetHandle handle) noexcept {
    onDestroyDescriptorSet(handle);
}

void Renderer::destroyRenderPass(RenderPassHandle handle) noexcept {
    onDestroyRenderPass(handle);
}

void Renderer::destroyFramebuffer(FramebufferHandle handle) noexcept {
    onDestroyFramebuffer(handle);
}

void Renderer::bind(DescriptorSetHandle handle, gerium_uint16_t binding, BufferHandle buffer) noexcept {
    onBind(handle, binding, buffer);
}

void Renderer::bind(DescriptorSetHandle handle, gerium_uint16_t binding, gerium_utf8_t name) noexcept {
    onBind(handle, binding, name);
}

gerium_data_t Renderer::mapBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size) noexcept {
    return onMapBuffer(handle, offset, size);
}

void Renderer::unmapBuffer(BufferHandle handle) noexcept {
    onUnmapBuffer(handle);
}

bool Renderer::newFrame() {
    return onNewFrame();
}

void Renderer::render(FrameGraph& frameGraph) {
    onRender(frameGraph);
}

void Renderer::present() {
    onPresent();
}

Profiler* Renderer::getProfiler() noexcept {
    return onGetProfiler();
}

void Renderer::getSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept {
    onGetSwapchainSize(width, height);
}

} // namespace gerium

using namespace gerium;

gerium_renderer_t gerium_renderer_reference(gerium_renderer_t renderer) {
    assert(renderer);
    renderer->reference();
    return renderer;
}

void gerium_renderer_destroy(gerium_renderer_t renderer) {
    if (renderer) {
        renderer->destroy();
    }
}

gerium_bool_t gerium_renderer_get_profiler_enable(gerium_renderer_t renderer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->getProfilerEnable();
}

void gerium_renderer_set_profiler_enable(gerium_renderer_t renderer, gerium_bool_t enable) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->setProfilerEnable(enable);
}

gerium_result_t gerium_renderer_create_buffer(gerium_renderer_t renderer,
                                              gerium_buffer_usage_flags_t buffer_usage,
                                              gerium_bool_t dynamic,
                                              gerium_utf8_t name,
                                              gerium_cdata_t data,
                                              gerium_uint32_t size,
                                              gerium_buffer_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(handle);

    BufferCreation bc;
    bc.set(buffer_usage, dynamic ? ResourceUsageType::Dynamic : ResourceUsageType::Immutable, size)
        .setName(name)
        .setInitialData((void*) data);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createBuffer(bc);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_texture(gerium_renderer_t renderer,
                                               const gerium_texture_creation_t* creation,
                                               gerium_texture_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(creation);
    GERIUM_ASSERT_ARG(handle);

    TextureCreation tc;
    tc.setSize(creation->width, creation->height, creation->depth)
        .setFlags(creation->mipmaps, false, false)
        .setFormat(creation->format, creation->type)
        .setData((void*) creation->data)
        .setName(creation->name);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createTexture(tc);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_material(gerium_renderer_t renderer,
                                                gerium_frame_graph_t frame_graph,
                                                gerium_utf8_t name,
                                                gerium_uint32_t pipeline_count,
                                                const gerium_pipeline_t* pipelines,
                                                gerium_material_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_ASSERT_ARG(pipeline_count > 0);
    GERIUM_ASSERT_ARG(pipelines);
    GERIUM_ASSERT_ARG(handle);
    for (gerium_uint32_t i = 0; i < pipeline_count; ++i) {
        GERIUM_ASSERT_ARG(pipelines[i].rasterization);
        GERIUM_ASSERT_ARG(pipelines[i].depth_stencil);
        GERIUM_ASSERT_ARG(pipelines[i].color_blend);
    }

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createMaterial(
            *alias_cast<FrameGraph*>(frame_graph), name, pipeline_count, pipelines);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_descriptor_set(gerium_renderer_t renderer, gerium_descriptor_set_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createDescriptorSet();
    GERIUM_END_SAFE_BLOCK
}

void gerium_renderer_destroy_buffer(gerium_renderer_t renderer, gerium_buffer_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyBuffer({ handle.unused });
}

void gerium_renderer_destroy_texture(gerium_renderer_t renderer, gerium_texture_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyTexture({ handle.unused });
}

void gerium_renderer_destroy_material(gerium_renderer_t renderer, gerium_material_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyMaterial({ handle.unused });
}

void gerium_renderer_destroy_descriptor_set(gerium_renderer_t renderer, gerium_descriptor_set_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyDescriptorSet({ handle.unused });
}

void gerium_renderer_bind_buffer(gerium_renderer_t renderer,
                                 gerium_descriptor_set_h handle,
                                 gerium_uint16_t binding,
                                 gerium_buffer_h buffer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.unused }, binding, BufferHandle{ buffer.unused });
}

void gerium_renderer_bind_resource(gerium_renderer_t renderer,
                                   gerium_descriptor_set_h handle,
                                   gerium_uint16_t binding,
                                   gerium_utf8_t name) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.unused }, binding, name);
}

gerium_data_t gerium_renderer_map_buffer(gerium_renderer_t renderer,
                                         gerium_buffer_h handle,
                                         gerium_uint32_t offset,
                                         gerium_uint32_t size) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->mapBuffer({ handle.unused }, offset, size);
}

void gerium_renderer_unmap_buffer(gerium_renderer_t renderer, gerium_buffer_h handle) {
    assert(renderer);
    alias_cast<Renderer*>(renderer)->unmapBuffer({ handle.unused });
}

gerium_result_t gerium_renderer_new_frame(gerium_renderer_t renderer) {
    assert(renderer);
    GERIUM_BEGIN_SAFE_BLOCK
        if (!alias_cast<Renderer*>(renderer)->newFrame()) {
            return GERIUM_RESULT_SKIP_FRAME;
        }
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_render(gerium_renderer_t renderer, gerium_frame_graph_t frame_graph) {
    assert(renderer);
    GERIUM_ASSERT_ARG(frame_graph);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->render(*alias_cast<FrameGraph*>(frame_graph));
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_present(gerium_renderer_t renderer) {
    assert(renderer);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->present();
    GERIUM_END_SAFE_BLOCK
}
