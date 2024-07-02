#ifndef GERIUM_WINDOWS_VULKAN_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_DEVICE_HPP

#include "../Application.hpp"
#include "../Gerium.hpp"
#include "../Logger.hpp"
#include "../StringPool.hpp"
#include "Resources.hpp"
#include "Utils.hpp"

namespace gerium::vulkan {

class Device {
public:
    virtual ~Device();

    void create(Application* application, gerium_uint32_t version, bool enableValidations);

    TextureHandle createTexture(const TextureCreation& creation);

    void destroyTexture(TextureHandle handle);

protected:
    VkInstance instance() const noexcept {
        return _instance;
    }

    const vk::DispatchLoaderDynamic& vkTable() const noexcept {
        return _vkTable;
    }

private:
    enum class ResourceType {
        Buffer,
        Texture,
        Sampler,
        RenderPass,
        Framebuffer,
        Shader,
        DescriptorSet,
        DescriptorSetLayout,
        Pipeline
    };

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

    struct Swapchain {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct ResourceDeletion {
        ResourceType type;
        uint32_t frame;
        Handle handle;
    };

    void createInstance(gerium_utf8_t appName, gerium_uint32_t version);
    void createSurface(Application* application);
    void createPhysicalDevice();
    void createDevice();
    void createVmaAllocator();
    void createSwapchain(Application* application);

    void printValidationLayers();
    void printExtensions();
    void printPhysicalDevices();

    void deleteResources(bool forceDelete = false);
    void setObjectName(VkObjectType type, uint64_t handle, gerium_utf8_t name);
    int getPhysicalDeviceScore(VkPhysicalDevice device);
    QueueFamilies getQueueFamilies(VkPhysicalDevice device);
    Swapchain getSwapchain();

    std::vector<const char*> selectValidationLayers();
    std::vector<const char*> selectExtensions();
    std::vector<const char*> selectDeviceExtensions();
    VkPhysicalDevice selectPhysicalDevice();
    VkSurfaceFormatKHR selectSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR selectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
    VkExtent2D selectSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, Application* application);

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
    VmaAllocator _vmaAllocator{};
    VkSwapchainKHR _swapchain{};
    VkSurfaceFormatKHR _swapchainFormat{};
    VkExtent2D _swapchainExtent{};

    QueueFamilies _queueFamilies{};
    VkPhysicalDeviceProperties _deviceProperties{};
    VkPhysicalDeviceMemoryProperties _deviceMemProperties{};
    uint32_t _uboAlignment{};
    uint32_t _ssboAlignment{};
    bool _profilerSupported{};

    TexturePool _textures;

    std::queue<ResourceDeletion> _deletionQueue{};
};

} // namespace gerium::vulkan

#endif
