#include "MacOSVkRenderer.hpp"

namespace gerium::vulkan::macos {

MacOSVkRenderer::MacOSVkRenderer(gerium::macos::MacOSApplication* application) :
    VkRenderer(application, std::make_unique<MacOSDevice>()) {
    if (!application->isRunning()) {
        error(GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING);
    }
}

void MacOSVkRenderer::onInitialize(gerium_uint32_t version, bool debug) {
    VkRenderer::onInitialize(version, debug);
}

} // namespace gerium::vulkan::macos

gerium_result_t gerium_renderer_create(gerium_application_t application,
                                       gerium_uint32_t version,
                                       gerium_bool_t debug,
                                       gerium_renderer_t* renderer) {
    using namespace gerium;
    using namespace gerium::macos;
    using namespace gerium::vulkan::macos;
    assert(application);
    auto result = Object::create<MacOSVkRenderer>(*renderer, alias_cast<MacOSApplication*>(application));
    if (result != GERIUM_RESULT_SUCCESS) {
        return result;
    }
    return alias_cast<MacOSVkRenderer*>(*renderer)->initialize(version, debug != 0);
}