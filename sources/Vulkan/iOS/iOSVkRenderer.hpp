#ifndef GERIUM_WINDOWS_VULKAN_IOS_IOS_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_IOS_IOS_VK_RENDERER_HPP

#include "../VkRenderer.hpp"
#include "iOSDevice.hpp"

namespace gerium::vulkan::ios {

class iOSVkRenderer final : public VkRenderer {
public:
    explicit iOSVkRenderer(gerium::Application* application);

protected:
    void onInitialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) override;
};

} // namespace gerium::vulkan::ios

#endif
