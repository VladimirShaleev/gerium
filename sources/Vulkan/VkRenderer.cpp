#include "VkRenderer.hpp"

namespace gerium::vulkan {

VkRenderer::VkRenderer(Application* application) noexcept : _application(application) {
    gerium_logger_t logger = nullptr;
    if (auto result = gerium_logger_create("gerium:renderer", &logger); result != GERIUM_RESULT_SUCCESS) {
        error(result);
    }
    _logger = ObjectPtr(alias_cast<Logger*>(logger), false);
}

void VkRenderer::onInitialize(gerium_uint32_t version, bool debug) {
    _device.create(application()->getTitle(), version, debug);
}

Application* VkRenderer::application() noexcept {
    return _application.get();
}

Logger* VkRenderer::logger() noexcept {
    return _logger.get();
}

} // namespace gerium::vulkan
