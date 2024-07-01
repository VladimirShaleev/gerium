#include "VkRenderer.hpp"

namespace gerium::vulkan {

VkRenderer::VkRenderer(Application* application, std::unique_ptr<Device>&& device) noexcept :
    _application(application),
    _device(std::move(device)) {
}

void VkRenderer::onInitialize(gerium_uint32_t version, bool debug) {
    _device->create(application(), version, debug);
}

Application* VkRenderer::application() noexcept {
    return _application.get();
}

} // namespace gerium::vulkan
