#ifndef GERIUM_WINDOWS_VULKAN_ANDROID_ANDROID_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_ANDROID_ANDROID_VK_RENDERER_HPP

#include "../../Android/AndroidApplication.hpp"
#include "../VkRenderer.hpp"
#include "AndroidDevice.hpp"

namespace gerium::vulkan::android {

class AndroidVkRenderer final : public VkRenderer {
public:
    explicit AndroidVkRenderer(gerium::android::AndroidApplication* application);

protected:
    void onInitialize(gerium_uint32_t version, bool debug) override;
};

} // namespace gerium::vulkan::android

#endif
