#include "LinuxVkRenderer.hpp"
#include "LinuxDevice.hpp"

namespace gerium::vulkan::linux {

LinuxVkRenderer::LinuxVkRenderer(gerium::Application* application) :
    VkRenderer(application, createObjectPtr<LinuxDevice, gerium::vulkan::Device>()) {
}

void LinuxVkRenderer::onInitialize(gerium_feature_flags_t features, const gerium_renderer_options_t& options) {
    VkRenderer::onInitialize(features, options);
}

} // namespace gerium::vulkan::linux

gerium_result_t gerium_renderer_create(gerium_application_t application,
                                       gerium_feature_flags_t features,
                                       const gerium_renderer_options_t* options,
                                       gerium_renderer_t* renderer) {
    using namespace gerium;
    using namespace gerium::vulkan::linux;
    assert(application);
    auto result = Object::create<LinuxVkRenderer>(*renderer, alias_cast<Application*>(application));
    if (result != GERIUM_RESULT_SUCCESS) {
        return result;
    }
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<LinuxVkRenderer*>(*renderer)->initialize(features, options);
    GERIUM_END_SAFE_BLOCK
}
