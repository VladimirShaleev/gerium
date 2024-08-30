#ifndef GERIUM_WINDOWS_VULKAN_MAC_OS_MAC_OS_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_MAC_OS_MAC_OS_VK_RENDERER_HPP

#include "../../MacOS/MacOSApplication.hpp"
#include "../VkRenderer.hpp"
#include "MacOSDevice.hpp"

namespace gerium::vulkan::macos {

class MacOSVkRenderer final : public VkRenderer {
public:
    explicit MacOSVkRenderer(gerium::macos::MacOSApplication* application);

protected:
    void onInitialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) override;
};

} // namespace gerium::vulkan::macos

#endif
