#ifndef GERIUM_WINDOWS_VULKAN_MAC_OS_MAC_OS_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_MAC_OS_MAC_OS_DEVICE_HPP

#include "../../MacOS/MacOSApplication.hpp"
#include "../Device.hpp"

namespace gerium::vulkan::macos {

class MacOSDevice : public Device {
private:
    VkInstanceCreateFlags onGetCreateInfoFlags() const noexcept override;
    const void* onGetNextCreateInfo(void* pNext) noexcept override;
    std::vector<const char*> onGetInstanceExtensions() const noexcept override;
    std::vector<const char*> onGetDeviceExtensions() const noexcept override;
    bool onNeedPostAcquireResize() const noexcept override;
    void onImGuiGetScaleAndFontSize(Application* application,
                                    gerium_float32_t& scale,
                                    gerium_float32_t& fontSize) const noexcept override;
    VkSurfaceKHR onCreateSurface(Application* application) const override;
};

} // namespace gerium::vulkan::macos

#endif
