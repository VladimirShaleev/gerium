#include "Renderer.hpp"

namespace gerium {

Renderer::Renderer() noexcept {
}

gerium_result_t Renderer::initialize(gerium_uint32_t version) noexcept {
    return invoke<Renderer>([version](auto obj) {
        obj->onInitialize(version);
    });
}

} // namespace gerium

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
