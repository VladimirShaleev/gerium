#ifndef GERIUM_WINDOWS_VULKAN_ANDROID_ANDROID_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_ANDROID_ANDROID_DEVICE_HPP

#include "../../Android/AndroidApplication.hpp"
#include "../Device.hpp"

namespace gerium::vulkan::android {

class AndroidDevice : public Device {
private:
    const char* onGetSurfaceExtension() const noexcept override;
    VkSurfaceKHR onCreateSurface(Application* application) const override;
};

} // namespace gerium::vulkan::android

#endif
