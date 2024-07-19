#include "Renderer.hpp"
#include "FrameGraph.hpp"

namespace gerium {

Renderer::Renderer() noexcept {
}

void Renderer::initialize(gerium_uint32_t version, bool debug) {
    onInitialize(version, debug);
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

RenderPassHandle Renderer::createRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    return onCreateRenderPass(frameGraph, node);
}

FramebufferHandle Renderer::createFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    return onCreateFramebuffer(frameGraph, node);
}

void Renderer::destroyTexture(gerium_texture_h handle) noexcept {
    onDestroyTexture({ handle.unused });
}

bool Renderer::newFrame() {
    return onNewFrame();
}

void Renderer::render(const FrameGraph& frameGraph) {
    onRender(frameGraph);
}

void Renderer::present() {
    onPresent();
}

Profiler* Renderer::getProfiler() noexcept {
    return onGetProfiler();
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

gerium_result_t gerium_renderer_create_buffer_from_data(gerium_renderer_t renderer,
                                                        gerium_utf8_t name,
                                                        gerium_cdata_t data,
                                                        gerium_uint32_t size,
                                                        gerium_buffer_h* handle) {
    assert(renderer);
    assert(data);
    assert(handle);

    BufferCreation bc;
    bc.set(BufferUsageFlags::Vertex | BufferUsageFlags::Index | BufferUsageFlags::Uniform,
           ResourceUsageType::Immutable,
           size)
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
    assert(creation);
    assert(handle);

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
    assert(frame_graph);
    assert(name);
    assert(pipeline_count > 0);
    assert(pipelines);
    assert(handle);

    GERIUM_BEGIN_SAFE_BLOCK
        *handle = alias_cast<Renderer*>(renderer)->createMaterial(
            *alias_cast<FrameGraph*>(frame_graph), name, pipeline_count, pipelines);
    GERIUM_END_SAFE_BLOCK
}

void gerium_renderer_destroy_texture(gerium_renderer_t renderer, gerium_texture_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyTexture(handle);
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
    assert(frame_graph);
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
