#include "iOSVkRenderer.hpp"

namespace gerium::vulkan::ios {

iOSVkRenderer::iOSVkRenderer(gerium::Application* application) :
    VkRenderer(application, createObjectPtr<iOSDevice, gerium::vulkan::Device>()) {
}

void iOSVkRenderer::onInitialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) {
    VkRenderer::onInitialize(features, version, debug);
}

} // namespace gerium::vulkan::ios

gerium_result_t gerium_renderer_create(gerium_application_t application,
                                       gerium_feature_flags_t features,
                                       gerium_uint32_t version,
                                       gerium_bool_t debug,
                                       gerium_renderer_t* renderer) {
    using namespace gerium;
    using namespace gerium::vulkan::ios;
    assert(application);
    auto result = Object::create<iOSVkRenderer>(*renderer, alias_cast<Application*>(application));
    if (result != GERIUM_RESULT_SUCCESS) {
        return result;
    }
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<iOSVkRenderer*>(*renderer)->initialize(features, version, debug != 0);
    GERIUM_END_SAFE_BLOCK
}
