#ifndef GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP

#include "../Renderer.hpp"

namespace gerium::vulkan {

class VkRenderer : public Renderer {
public:
    VkRenderer();

protected:
    void onInitialize() override;
};

} // namespace gerium::vulkan

#endif
