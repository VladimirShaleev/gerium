#include "Renderer.hpp"
#include "FrameGraph.hpp"

namespace gerium {

Renderer::Renderer() noexcept {
}

gerium_result_t Renderer::initialize(gerium_uint32_t version, bool debug) noexcept {
    return invoke<Renderer>([version, debug](auto obj) {
        obj->onInitialize(version, debug);
    });
}

gerium_result_t Renderer::createBuffer(const gerium_buffer_creation_t& creation, gerium_buffer_h& handle) noexcept {
    return invoke<Renderer>([&creation, &handle](auto obj) {
        handle = obj->onCreateBuffer(creation);
    });
}

gerium_result_t Renderer::createTexture(const TextureCreation& creation, TextureHandle& handle) noexcept {
    return invoke<Renderer>([&creation, &handle](auto obj) {
        handle = obj->onCreateTexture(creation);
    });
}

gerium_result_t Renderer::createRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node, RenderPassHandle& handle) noexcept {
    return invoke<Renderer>([&frameGraph, node, &handle](auto obj) {
        handle = obj->onCreateRenderPass(frameGraph, node);
    });
}

gerium_result_t Renderer::createFramebuffer(const FrameGraph& frameGraph,
                                            const FrameGraphNode* node,
                                            FramebufferHandle& handle) noexcept {
    return invoke<Renderer>([&frameGraph, node, &handle](auto obj) {
        handle = obj->onCreateFramebuffer(frameGraph, node);
    });
}

void Renderer::destroyTexture(gerium_texture_h handle) noexcept {
    onDestroyTexture({ handle.unused });
}

gerium_result_t Renderer::newFrame() noexcept {
    bool drawFrame = false;

    auto result = invoke<Renderer>([&drawFrame](auto obj) {
        drawFrame = obj->onNewFrame();
    });
    if (result != GERIUM_RESULT_SUCCESS) {
        return result;
    }
    return drawFrame ? GERIUM_RESULT_SUCCESS : GERIUM_RESULT_SKIP_FRAME;
}

gerium_result_t Renderer::present() noexcept {
    return invoke<Renderer>([](auto obj) {
        obj->onPresent();
    });
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

gerium_result_t gerium_renderer_create_buffer(gerium_renderer_t renderer,
                                              const gerium_buffer_creation_t* creation,
                                              gerium_buffer_h* handle) {
    assert(renderer);
    assert(creation);
    assert(handle);
    return alias_cast<Renderer*>(renderer)->createBuffer(*creation, *handle);
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
    TextureHandle texture;
    auto result = alias_cast<Renderer*>(renderer)->createTexture(tc, texture);
    *handle = texture;
    return result;
}

void gerium_renderer_destroy_texture(gerium_renderer_t renderer, gerium_texture_h handle) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->destroyTexture(handle);
}

gerium_result_t gerium_renderer_new_frame(gerium_renderer_t renderer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->newFrame();
}

gerium_result_t gerium_renderer_present(gerium_renderer_t renderer) {
    assert(renderer);
    return alias_cast<Renderer*>(renderer)->present();
}
