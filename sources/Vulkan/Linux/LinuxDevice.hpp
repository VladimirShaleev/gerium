#ifndef GERIUM_WINDOWS_VULKAN_LINUX_LINUX_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_LINUX_LINUX_DEVICE_HPP

#include "../Device.hpp"

namespace gerium::vulkan::linux {

class LinuxDevice : public Device {
private:
    std::vector<const char*> onGetInstanceExtensions() const noexcept override;
    VkSurfaceKHR onCreateSurface(Application* application) const override;
};

} // namespace gerium::vulkan::windows

#endif
