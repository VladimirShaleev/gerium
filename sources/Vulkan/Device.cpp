#include "Device.hpp"

using namespace std::string_view_literals;

namespace gerium::vulkan {

void Device::create(gerium_utf8_t appName, gerium_uint32_t version, bool enableValidations) {
    gerium_logger_t logger = nullptr;
    if (auto result = gerium_logger_create("gerium:renderer:vulkan", &logger); result != GERIUM_RESULT_SUCCESS) {
        throw Exception(result);
    }
    _logger = ObjectPtr(alias_cast<Logger*>(logger), false);

    createInstance(appName, version, enableValidations);
}

void Device::createInstance(gerium_utf8_t appName, gerium_uint32_t version, bool enableValidations) {
#ifndef __APPLE__
    _vkTable.init();
#else
    _vkTable.init(vkGetInstanceProcAddr);
#endif

    _enableValidations = enableValidations;

    printValidationLayers();
    printExtensions();
}

void Device::printValidationLayers() {
    if (_enableValidations) {
        uint32_t count = 0;
        check(_vkTable.vkEnumerateInstanceLayerProperties(&count, nullptr));

        std::vector<VkLayerProperties> layers;
        layers.resize(count);
        check(_vkTable.vkEnumerateInstanceLayerProperties(&count, layers.data()));

        logger()->print(GERIUM_LOGGER_LEVEL_VERBOSE, "Supported validation layers:");
        for (const auto& layer : layers) {
            std::ostringstream ss;
            ss << "    "sv << layer.layerName << "(ver "sv << VK_API_VERSION_VARIANT(layer.specVersion) << '.'
               << VK_API_VERSION_MAJOR(layer.specVersion) << '.' << VK_API_VERSION_MINOR(layer.specVersion) << '.'
               << VK_API_VERSION_PATCH(layer.specVersion) << ", impl ver "sv << layer.implementationVersion << ") -- "sv
               << layer.description;
            const auto str = ss.str();
            logger()->print(GERIUM_LOGGER_LEVEL_VERBOSE, str.c_str());
        }
        logger()->print(GERIUM_LOGGER_LEVEL_VERBOSE, "");
    }
}

void Device::printExtensions() {
    if (_enableValidations) {
        uint32_t count = 0;
        check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

        std::vector<VkExtensionProperties> extensions;
        extensions.resize(count);
        check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));

        logger()->print(GERIUM_LOGGER_LEVEL_VERBOSE, "Supported extensions:");
        for (const auto& extension : extensions) {
            std::ostringstream ss;
            ss << "    "sv << extension.extensionName << " (ver "sv << extension.specVersion << ')';
            const auto str = ss.str();
            logger()->print(GERIUM_LOGGER_LEVEL_VERBOSE, str.c_str());
        }
        logger()->print(GERIUM_LOGGER_LEVEL_VERBOSE, "");
    }
}

Logger* Device::logger() noexcept {
    return _logger.get();
}

} // namespace gerium::vulkan
