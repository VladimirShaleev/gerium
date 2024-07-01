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
    struct QueueFamily {
        uint8_t index;
        uint8_t queue;
        uint8_t timestampValidBits;

        QueueFamily(int i, int q, uint32_t t) noexcept :
            index(uint8_t(i)),
            queue(uint8_t(q)),
            timestampValidBits(uint8_t(t)) {
        }
    };

    struct QueueFamilies {
        std::optional<QueueFamily> graphic;
        std::optional<QueueFamily> compute;
        std::optional<QueueFamily> present;
        std::optional<QueueFamily> transfer;

        bool isComplete() const noexcept {
            return graphic.has_value() && compute.has_value() && present.has_value() && transfer.has_value();
        }
    };

    void createInstance(gerium_utf8_t appName, gerium_uint32_t version);
    void createSurface(Application* application);
    void createPhysicalDevice();
    void createDevice();

    void printValidationLayers();
    void printExtensions();
    void printPhysicalDevices();

    int getPhysicalDeviceScore(VkPhysicalDevice device);
    QueueFamilies getQueueFamilies(VkPhysicalDevice device);

    std::vector<const char*> selectValidationLayers();
    std::vector<const char*> selectExtensions();
    std::vector<const char*> selectDeviceExtensions();
    VkPhysicalDevice selectPhysicalDevice();

    std::vector<const char*> checkValidationLayers(const std::vector<const char*>& layers);
    std::vector<const char*> checkExtensions(const std::vector<const char*>& extensions);
    bool checkPhysicalDeviceExtensions(VkPhysicalDevice device, const std::vector<const char*>& extensions);

    void debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                void* pUserData);

    [[noreturn]] static void error(gerium_result_t result);

    virtual const char* onGetSurfaceExtension() const noexcept           = 0;
    virtual VkSurfaceKHR onCreateSurface(Application* application) const = 0;

    bool _enableValidations{};
    ObjectPtr<Logger> _logger;

    vk::DispatchLoaderDynamic _vkTable;
    VkInstance _instance{};
    VkSurfaceKHR _surface{};
    VkPhysicalDevice _physicalDevice{};
    VkDevice _device{};
    VkQueue _queueGraphic{};
    VkQueue _queueCompute{};
    VkQueue _queuePresent{};
    VkQueue _queueTransfer{};

    QueueFamilies _queueFamilies{};
    VkPhysicalDeviceProperties _deviceProperties{};
    VkPhysicalDeviceMemoryProperties _deviceMemProperties{};
    uint32_t _uboAlignment{};
    uint32_t _ssboAlignment{};
    bool _profilerSupported{};
};

} // namespace gerium::vulkan

#endif
