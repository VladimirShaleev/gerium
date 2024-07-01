#ifndef GERIUM_WINDOWS_VULKAN_WINDOWS_WIN32_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_WINDOWS_WIN32_DEVICE_HPP

#include "../../Windows/Win32Application.hpp"
#include "../Device.hpp"

namespace gerium::vulkan::windows {

class Win32Device : public Device {
private:
    const char* onGetSurfaceExtension() const noexcept override;
    VkSurfaceKHR onCreateSurface(Application* application) const override;
};

} // namespace gerium::vulkan::windows

#endif
