#include "Win32VkRenderer.hpp"

namespace gerium::vulkan::windows {

Win32VkRenderer::Win32VkRenderer() {
}

} // namespace gerium::vulkan::windows

gerium_result_t gerium_renderer_create(gerium_application_t application, gerium_renderer_t* renderer) {
    using namespace gerium;
    using namespace gerium::vulkan::windows;
    assert(application);
    return Object::create<Win32VkRenderer>(*renderer);
}
