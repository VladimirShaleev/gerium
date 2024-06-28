#include "Device.hpp"

namespace gerium::vulkan {

void Device::create(gerium_utf8_t appName, gerium_uint32_t version, bool enableValidations) {
    createInstance(appName, version, enableValidations);
}

void Device::createInstance(gerium_utf8_t appName, gerium_uint32_t version, bool enableValidations) {
#ifndef __APPLE__
    _vkTable.init();
#else
    _vkTable.init(vkGetInstanceProcAddr);
#endif

    _enableValidations = enableValidations;
}

} // namespace gerium::vulkan
