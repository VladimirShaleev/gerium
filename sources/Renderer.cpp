#include "Renderer.hpp"

namespace gerium {

Renderer::Renderer() noexcept {
}

gerium_result_t Renderer::initialize(gerium_uint32_t version, bool debug) noexcept {
    return invoke<Renderer>([version, debug](auto obj) {
        obj->onInitialize(version, debug);
    });
}

gerium_result_t Renderer::createTexture(const gerium_texture_creation_t& creation, gerium_texture_h& handle) noexcept {
    return invoke<Renderer>([&creation, &handle](auto obj) {
        handle = obj->onCreateTexture(creation);
    });
}

void Renderer::destroyTexture(gerium_texture_h handle) noexcept {
    onDestroyTexture({ handle.unused });
}

gerium_result_t Renderer::newFrame() noexcept {
    return invoke<Renderer>([](auto obj) {
        obj->onNewFrame();
    });
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

gerium_result_t gerium_renderer_create_texture(gerium_renderer_t renderer,
                                               const gerium_texture_creation_t* creation,
                                               gerium_texture_h* handle) {
    assert(renderer);
    assert(creation);
    return alias_cast<Renderer*>(renderer)->createTexture(*creation, *handle);
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
