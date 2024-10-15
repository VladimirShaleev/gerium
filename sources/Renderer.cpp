#include "Renderer.hpp"
#include "FrameGraph.hpp"

namespace gerium {

Renderer::Renderer() noexcept {
}

void Renderer::initialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) {
    onInitialize(features, version, debug);
}

gerium_feature_flags_t Renderer::getEnabledFeatures() const noexcept {
    return onGetEnabledFeatures();
}

bool Renderer::getProfilerEnable() const noexcept {
    return onGetProfilerEnable();
}

void Renderer::setProfilerEnable(bool enable) noexcept {
    onSetProfilerEnable(enable);
}

bool Renderer::isSupportedFormat(gerium_format_t format) noexcept {
    return onIsSupportedFormat(format);
}

void Renderer::getTextureInfo(TextureHandle handle, gerium_texture_info_t& info) noexcept {
    onGetTextureInfo(handle, info);
}

BufferHandle Renderer::createBuffer(const BufferCreation& creation) {
    return onCreateBuffer(creation);
}

TextureHandle Renderer::createTexture(const TextureCreation& creation) {
    return onCreateTexture(creation);
}

TechniqueHandle Renderer::createTechnique(const FrameGraph& frameGraph,
                                          gerium_utf8_t name,
                                          gerium_uint32_t pipelineCount,
                                          const gerium_pipeline_t* pipelines) {
    return onCreateTechnique(frameGraph, name, pipelineCount, pipelines);
}

DescriptorSetHandle Renderer::createDescriptorSet(bool global) {
    return onCreateDescriptorSet(global);
}

RenderPassHandle Renderer::createRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    return onCreateRenderPass(frameGraph, node);
}

FramebufferHandle Renderer::createFramebuffer(const FrameGraph& frameGraph,
                                              const FrameGraphNode* node,
                                              gerium_uint32_t textureIndex) {
    return onCreateFramebuffer(frameGraph, node, textureIndex);
}

void Renderer::asyncUploadTextureData(TextureHandle handle,
                                      gerium_cdata_t textureData,
                                      gerium_texture_loaded_func_t callback,
                                      gerium_data_t data) {
    onAsyncUploadTextureData(handle, textureData, callback, data);
}

void Renderer::textureSampler(TextureHandle handle,
                              gerium_filter_t minFilter,
                              gerium_filter_t magFilter,
                              gerium_filter_t mipFilter,
                              gerium_address_mode_t addressModeU,
                              gerium_address_mode_t addressModeV,
                              gerium_address_mode_t addressModeW) {
    onTextureSampler(handle, minFilter, magFilter, mipFilter, addressModeU, addressModeV, addressModeW);
}

BufferHandle Renderer::getBuffer(gerium_utf8_t resource, bool fromOutput) {
    return onGetBuffer(resource, fromOutput);
}

TextureHandle Renderer::getTexture(gerium_utf8_t resource, bool fromOutput, bool fromPreviousFrame) {
    return onGetTexture(resource, fromOutput, fromPreviousFrame);
}

void Renderer::destroyBuffer(BufferHandle handle) noexcept {
    onDestroyBuffer(handle);
}

void Renderer::destroyTexture(TextureHandle handle) noexcept {
    onDestroyTexture(handle);
}

void Renderer::destroyTechnique(TechniqueHandle handle) noexcept {
    onDestroyTechnique(handle);
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

void Renderer::bind(DescriptorSetHandle handle,
                    gerium_uint16_t binding,
                    gerium_uint16_t element,
                    TextureHandle texture) noexcept {
    onBind(handle, binding, element, texture);
}

void Renderer::bind(DescriptorSetHandle handle, gerium_uint16_t binding, gerium_utf8_t resourceInput) noexcept {
    onBind(handle, binding, resourceInput);
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

gerium_feature_flags_t gerium_renderer_get_enabled_features(gerium_renderer_t renderer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->getEnabledFeatures();
}

gerium_bool_t gerium_renderer_get_profiler_enable(gerium_renderer_t renderer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->getProfilerEnable();
}

void gerium_renderer_set_profiler_enable(gerium_renderer_t renderer, gerium_bool_t enable) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->setProfilerEnable(enable);
}

gerium_bool_t gerium_renderer_is_supported_format(gerium_renderer_t renderer, gerium_format_t format) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->isSupportedFormat(format);
}

void gerium_renderer_get_texture_info(gerium_renderer_t renderer,
                                      gerium_texture_h handle,
                                      gerium_texture_info_t* info) {
    assert(renderer);
    assert(info);
    return alias_cast<Renderer*>(renderer)->getTextureInfo({ handle.index }, *info);
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
                                               const gerium_texture_info_t* info,
                                               gerium_cdata_t data,
                                               gerium_texture_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(info);
    GERIUM_ASSERT_ARG(handle);

    TextureCreation tc;
    tc.setSize(info->width, info->height, info->depth)
        .setFlags(info->mipmaps, false, false)
        .setFormat(info->format, info->type)
        .setData((void*) data)
        .setName(info->name);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createTexture(tc);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_technique(gerium_renderer_t renderer,
                                                 gerium_frame_graph_t frame_graph,
                                                 gerium_utf8_t name,
                                                 gerium_uint32_t pipeline_count,
                                                 const gerium_pipeline_t* pipelines,
                                                 gerium_technique_h* handle) {
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
        *handle = alias_cast<Renderer*>(renderer)->createTechnique(
            *alias_cast<FrameGraph*>(frame_graph), name, pipeline_count, pipelines);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_create_descriptor_set(gerium_renderer_t renderer,
                                                      gerium_bool_t global,
                                                      gerium_descriptor_set_h* handle) {
    assert(renderer);
    GERIUM_ASSERT_ARG(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createDescriptorSet(global);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_async_upload_texture_data(gerium_renderer_t renderer,
                                                          gerium_texture_h handle,
                                                          gerium_cdata_t texture_data,
                                                          gerium_texture_loaded_func_t callback,
                                                          gerium_data_t data) {
    assert(renderer);
    GERIUM_ASSERT_ARG(texture_data);

    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->asyncUploadTextureData({ handle.index }, texture_data, callback, data);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_texture_sampler(gerium_renderer_t renderer,
                                                gerium_texture_h handle,
                                                gerium_filter_t min_filter,
                                                gerium_filter_t mag_filter,
                                                gerium_filter_t mip_filter,
                                                gerium_address_mode_t address_mode_u,
                                                gerium_address_mode_t address_mode_v,
                                                gerium_address_mode_t address_mode_w) {
    assert(renderer);

    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Renderer*>(renderer)->textureSampler(
            { handle.index }, min_filter, mag_filter, mip_filter, address_mode_u, address_mode_v, address_mode_w);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_get_buffer(gerium_renderer_t renderer,
                                           gerium_utf8_t resource,
                                           gerium_bool_t from_output,
                                           gerium_buffer_h* handle) {
    assert(renderer);
    assert(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->getBuffer(resource, from_output);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_renderer_get_texture(gerium_renderer_t renderer,
                                            gerium_utf8_t resource,
                                            gerium_bool_t from_output,
                                            gerium_bool_t from_previous_frame,
                                            gerium_texture_h* handle) {
    assert(renderer);
    assert(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->getTexture(resource, from_output, from_previous_frame);
    GERIUM_END_SAFE_BLOCK
}

void gerium_renderer_destroy_buffer(gerium_renderer_t renderer, gerium_buffer_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyBuffer({ handle.index });
}

void gerium_renderer_destroy_texture(gerium_renderer_t renderer, gerium_texture_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyTexture({ handle.index });
}

void gerium_renderer_destroy_technique(gerium_renderer_t renderer, gerium_technique_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyTechnique({ handle.index });
}

void gerium_renderer_destroy_descriptor_set(gerium_renderer_t renderer, gerium_descriptor_set_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyDescriptorSet({ handle.index });
}

void gerium_renderer_bind_buffer(gerium_renderer_t renderer,
                                 gerium_descriptor_set_h handle,
                                 gerium_uint16_t binding,
                                 gerium_buffer_h buffer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.index }, binding, BufferHandle{ buffer.index });
}

void gerium_renderer_bind_texture(gerium_renderer_t renderer,
                                  gerium_descriptor_set_h handle,
                                  gerium_uint16_t binding,
                                  gerium_uint16_t element,
                                  gerium_texture_h texture) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.index }, binding, element, TextureHandle{ texture.index });
}

void gerium_renderer_bind_resource(gerium_renderer_t renderer,
                                   gerium_descriptor_set_h handle,
                                   gerium_uint16_t binding,
                                   gerium_utf8_t resource_input) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->bind({ handle.index }, binding, resource_input);
}

gerium_data_t gerium_renderer_map_buffer(gerium_renderer_t renderer,
                                         gerium_buffer_h handle,
                                         gerium_uint32_t offset,
                                         gerium_uint32_t size) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->mapBuffer({ handle.index }, offset, size);
}

void gerium_renderer_unmap_buffer(gerium_renderer_t renderer, gerium_buffer_h handle) {
    assert(renderer);
    alias_cast<Renderer*>(renderer)->unmapBuffer({ handle.index });
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
