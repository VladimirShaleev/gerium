#include "Win32Device.hpp"
#include "../../Windows/Win32Application.hpp"

namespace gerium::vulkan::windows {

std::vector<const char*> Win32Device::onGetInstanceExtensions() const noexcept {
    return { VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
}

VkSurfaceKHR Win32Device::onCreateSurface(Application* application) const {
    auto win32App = alias_cast<gerium::windows::Win32Application*>(application);

    VkWin32SurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    createInfo.hinstance = win32App->hInstance();
    createInfo.hwnd      = win32App->hWnd();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    check(vkTable().vkCreateWin32SurfaceKHR(instance(), &createInfo, getAllocCalls(), &surface));
    return surface;
}

} // namespace gerium::vulkan::windows
