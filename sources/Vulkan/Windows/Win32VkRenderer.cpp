#include "Win32VkRenderer.hpp"

namespace gerium::vulkan::windows {

Win32VkRenderer::Win32VkRenderer(gerium::windows::Win32Application* application) {
    if (!application->isRunning()) {
        error(GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING);
    }
}

} // namespace gerium::vulkan::windows

gerium_result_t gerium_renderer_create(gerium_application_t application, gerium_renderer_t* renderer) {
    using namespace gerium;
    using namespace gerium::windows;
    using namespace gerium::vulkan::windows;
    assert(application);
    return Object::create<Win32VkRenderer>(*renderer, alias_cast<Win32Application*>(application));
}
