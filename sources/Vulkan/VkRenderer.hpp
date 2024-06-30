#ifndef GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP

#include "../Application.hpp"
#include "../Logger.hpp"
#include "../Renderer.hpp"
#include "Device.hpp"

namespace gerium::vulkan {

class VkRenderer : public Renderer {
public:
    VkRenderer(Application* application) noexcept;

protected:
    void onInitialize() override;

    Application* application() noexcept;

    Logger* logger() noexcept;

private:
    ObjectPtr<Application> _application;
    ObjectPtr<Logger> _logger;
    Device _device;
};

} // namespace gerium::vulkan

#endif
