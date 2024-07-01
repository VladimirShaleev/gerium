#ifndef GERIUM_WINDOWS_VULKAN_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_DEVICE_HPP

#include "../Application.hpp"
#include "../Gerium.hpp"
#include "../Logger.hpp"
#include "Utils.hpp"

namespace gerium::vulkan {

class Device {
public:
    virtual ~Device();

    void create(Application* application, gerium_uint32_t version, bool enableValidations);

protected:
    VkInstance instance() const noexcept {
        return _instance;
    }

    const vk::DispatchLoaderDynamic& vkTable() const noexcept {
        return _vkTable;
    }

private:
    void createInstance(gerium_utf8_t appName, gerium_uint32_t version);
    void createSurface(Application* application);
    void createPhysicalDevice();

    void printValidationLayers();
    void printExtensions();
    void printPhysicalDevices();

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

    virtual const char* onGetSurfaceExtension() const noexcept           = 0;
    virtual VkSurfaceKHR onCreateSurface(Application* application) const = 0;

    bool _enableValidations{};
    ObjectPtr<Logger> _logger;

    vk::DispatchLoaderDynamic _vkTable;
    VkInstance _instance{};
    VkSurfaceKHR _surface{};
};

} // namespace gerium::vulkan

#endif
