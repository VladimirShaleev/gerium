#include "Renderer.hpp"

namespace gerium {

Renderer::Renderer() noexcept {
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
