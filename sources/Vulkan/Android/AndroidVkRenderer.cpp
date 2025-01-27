#include "AndroidVkRenderer.hpp"

namespace gerium::vulkan::android {

AndroidVkRenderer::AndroidVkRenderer(gerium::android::AndroidApplication* application) :
    VkRenderer(application, createObjectPtr<AndroidDevice, gerium::vulkan::Device>()) {
}

void AndroidVkRenderer::onInitialize(gerium_feature_flags_t features, const gerium_renderer_options_t& options) {
    VkRenderer::onInitialize(features, options);
}

} // namespace gerium::vulkan::android

gerium_result_t gerium_renderer_create(gerium_application_t application,
                                       gerium_feature_flags_t features,
                                       const gerium_renderer_options_t* options,
                                       gerium_renderer_t* renderer) {
    using namespace gerium;
    using namespace gerium::android;
    using namespace gerium::vulkan::android;
    assert(application);
    auto result = Object::create<AndroidVkRenderer>(*renderer, alias_cast<AndroidApplication*>(application));
    if (result != GERIUM_RESULT_SUCCESS) {
        return result;
    }
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<AndroidVkRenderer*>(*renderer)->initialize(features, options);
    GERIUM_END_SAFE_BLOCK
}
