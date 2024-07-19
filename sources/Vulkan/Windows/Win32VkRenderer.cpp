#include "Win32VkRenderer.hpp"

namespace gerium::vulkan::windows {

Win32VkRenderer::Win32VkRenderer(gerium::windows::Win32Application* application) :
    VkRenderer(application, createObjectPtr<Win32Device, gerium::vulkan::Device>()) {
}

void Win32VkRenderer::onInitialize(gerium_uint32_t version, bool debug) {
    VkRenderer::onInitialize(version, debug);
}

} // namespace gerium::vulkan::windows

gerium_result_t gerium_renderer_create(gerium_application_t application,
                                       gerium_uint32_t version,
                                       gerium_bool_t debug,
                                       gerium_renderer_t* renderer) {
    using namespace gerium;
    using namespace gerium::windows;
    using namespace gerium::vulkan::windows;
    assert(application);
    auto result = Object::create<Win32VkRenderer>(*renderer, alias_cast<Win32Application*>(application));
    if (result != GERIUM_RESULT_SUCCESS) {
        return result;
    }
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Win32VkRenderer*>(*renderer)->initialize(version, debug != 0);
    GERIUM_END_SAFE_BLOCK
}
