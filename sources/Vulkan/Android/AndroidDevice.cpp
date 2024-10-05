#include "AndroidDevice.hpp"
#include "../../Android/AndroidApplication.hpp"

namespace gerium::vulkan::android {

std::vector<const char*> AndroidDevice::onGetInstanceExtensions() const noexcept {
    return { VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };
}

VkSurfaceKHR AndroidDevice::onCreateSurface(Application* application) const {
    auto androidApp = alias_cast<gerium::android::AndroidApplication*>(application);

    VkAndroidSurfaceCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR  };
    createInfo.window = androidApp->nativeWindow();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    check(vkTable().vkCreateAndroidSurfaceKHR(instance(), &createInfo, getAllocCalls(), &surface));
    return surface;
}

} // namespace gerium::vulkan::android
