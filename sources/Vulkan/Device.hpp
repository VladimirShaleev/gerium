#ifndef GERIUM_WINDOWS_VULKAN_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_DEVICE_HPP

#include "../Gerium.hpp"
#include "../Logger.hpp"
#include "Utils.hpp"

namespace gerium::vulkan {

class Device final {
public:
    ~Device();

    void create(gerium_utf8_t appName, gerium_uint32_t version, bool enableValidations);

private:
    void createInstance(gerium_utf8_t appName, gerium_uint32_t version);

    void printValidationLayers();
    void printExtensions();

    std::vector<const char*> selectValidationLayers();
    std::vector<const char*> selectExtensions();

    std::vector<const char*> checkValidationLayers(const std::vector<const char*>& layers);
    std::vector<const char*> checkExtensions(const std::vector<const char*>& extensions);

    void debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                void* pUserData);

    bool _enableValidations{};
    ObjectPtr<Logger> _logger;

    vk::DispatchLoaderDynamic _vkTable;
    VkInstance _instance{};
};

} // namespace gerium::vulkan

#endif
