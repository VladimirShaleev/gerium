#ifndef GERIUM_WINDOWS_VULKAN_IOS_IOS_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_IOS_IOS_DEVICE_HPP

#include "../Device.hpp"

namespace gerium::vulkan::ios {

class iOSDevice : public Device {
private:
    VkInstanceCreateFlags onGetCreateInfoFlags() const noexcept override;
    const void* onGetNextCreateInfo(void* pNext) noexcept override;
    std::vector<const char*> onGetInstanceExtensions() const noexcept override;
    std::vector<const char*> onGetDeviceExtensions() const noexcept override;
    bool onNeedPostAcquireResize() const noexcept override;
    VkSurfaceKHR onCreateSurface(Application* application) const override;
};

} // namespace gerium::vulkan::ios

#endif
