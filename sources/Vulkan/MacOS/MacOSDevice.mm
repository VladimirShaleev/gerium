#include "MacOSDevice.hpp"
#include "../../MacOS/MacOSApplication.hpp"

namespace gerium::vulkan::macos {

const char* MacOSDevice::onGetSurfaceExtension() const noexcept {
    return VK_EXT_METAL_SURFACE_EXTENSION_NAME;
}

VkSurfaceKHR MacOSDevice::onCreateSurface(Application* application) const {
    auto macosApp = alias_cast<gerium::macos::MacOSApplication*>(application);

    VkMetalSurfaceCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT };
    createInfo.pLayer = macosApp->layer();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    check(vkTable().vkCreateMetalSurfaceEXT(instance(), &createInfo, getAllocCalls(), &surface));
    return surface;
}

} // namespace gerium::vulkan::macos
