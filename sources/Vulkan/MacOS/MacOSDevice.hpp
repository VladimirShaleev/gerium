#ifndef GERIUM_WINDOWS_VULKAN_MAC_OS_MAC_OS_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_MAC_OS_MAC_OS_DEVICE_HPP

#include "../../MacOS/MacOSApplication.hpp"
#include "../Device.hpp"

namespace gerium::vulkan::macos {

class MacOSDevice : public Device {
private:
    const char* onGetSurfaceExtension() const noexcept override;
    VkSurfaceKHR onCreateSurface(Application* application) const override;
};

} // namespace gerium::vulkan::macos

#endif
