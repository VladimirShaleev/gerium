#ifndef GERIUM_WINDOWS_VULKAN_WINDOWS_WIN32_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_WINDOWS_WIN32_VK_RENDERER_HPP

#include "../../Windows/Win32Application.hpp"
#include "../VkRenderer.hpp"
#include "Win32Device.hpp"

namespace gerium::vulkan::windows {

class Win32VkRenderer final : public VkRenderer {
public:
    explicit Win32VkRenderer(gerium::windows::Win32Application* application);

protected:
    void onInitialize(gerium_feature_flags_t features, const gerium_renderer_options_t& options) override;
};

} // namespace gerium::vulkan::windows

#endif
