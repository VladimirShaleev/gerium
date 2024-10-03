#include "MacOSVkRenderer.hpp"

namespace gerium::vulkan::macos {

MacOSVkRenderer::MacOSVkRenderer(gerium::macos::MacOSApplication* application) :
    VkRenderer(application, createObjectPtr<MacOSDevice, gerium::vulkan::Device>()) {
}

void MacOSVkRenderer::onInitialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) {
    VkRenderer::onInitialize(features, version, debug);
}

} // namespace gerium::vulkan::macos

gerium_result_t gerium_renderer_create(gerium_application_t application,
                                       gerium_feature_flags_t features,
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
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<MacOSVkRenderer*>(*renderer)->initialize(features & ~GERIUM_FEATURE_BINDLESS, version, debug != 0);
    GERIUM_END_SAFE_BLOCK
}
