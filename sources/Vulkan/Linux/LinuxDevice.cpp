#include "LinuxDevice.hpp"
#include "../../Linux/LinuxApplication.hpp"

namespace gerium::vulkan::linux {

std::vector<const char*> LinuxDevice::onGetInstanceExtensions() const noexcept {
    return { VK_KHR_XCB_SURFACE_EXTENSION_NAME };
}

VkSurfaceKHR LinuxDevice::onCreateSurface(Application* application) const {
    auto linuxApp = alias_cast<gerium::linux::LinuxApplication*>(application);

    VkXcbSurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
    createInfo.connection = linuxApp->connection();
    createInfo.window     = linuxApp->window();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    check(vkTable().vkCreateXcbSurfaceKHR(instance(), &createInfo, getAllocCalls(), &surface));
    return surface;
}

} // namespace gerium::vulkan::linux
