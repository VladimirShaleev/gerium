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
