#ifndef GERIUM_WINDOWS_VULKAN_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_DEVICE_HPP

#include "../Gerium.hpp"
#include "../Logger.hpp"
#include "Utils.hpp"

namespace gerium::vulkan {

class Device final {
public:
    void create(gerium_utf8_t appName, gerium_uint32_t version, bool enableValidations);

private:
    void createInstance(gerium_utf8_t appName, gerium_uint32_t version, bool enableValidations);

    void printValidationLayers();
    void printExtensions();

    Logger* logger() noexcept;

    vk::DispatchLoaderDynamic _vkTable;

    bool _enableValidations;
    ObjectPtr<Logger> _logger;
};

} // namespace gerium::vulkan

#endif
