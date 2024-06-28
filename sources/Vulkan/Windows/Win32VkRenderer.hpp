#ifndef GERIUM_WINDOWS_VULKAN_WINDOWS_WIN32_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_WINDOWS_WIN32_VK_RENDERER_HPP

#include "../VkRenderer.hpp"

namespace gerium::vulkan::windows {

class Win32VkRenderer final : public VkRenderer {
public:
    Win32VkRenderer();
};

} // namespace gerium::vulkan

#endif
