#ifndef GERIUM_WINDOWS_VULKAN_LINUX_LINUX_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_LINUX_LINUX_VK_RENDERER_HPP

#include "../../Application.hpp"
#include "../VkRenderer.hpp"

namespace gerium::vulkan::linux {

class LinuxVkRenderer final : public VkRenderer {
public:
    explicit LinuxVkRenderer(gerium::Application* application);

protected:
    void onInitialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) override;
};

} // namespace gerium::vulkan::windows

#endif
