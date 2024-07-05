#ifndef GERIUM_WINDOWS_VULKAN_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_DEVICE_HPP

#include "../Application.hpp"
#include "../Gerium.hpp"
#include "../Logger.hpp"
#include "../StringPool.hpp"
#include "CommandBuffer.hpp"
#include "Resources.hpp"
#include "Utils.hpp"

namespace gerium::vulkan {

class Device {
public:
    virtual ~Device();

    void create(Application* application, gerium_uint32_t version, bool enableValidations);

    void newFrame();
    void submit(CommandBuffer* commandBuffer);
    void present();

    TextureHandle createTexture(const TextureCreation& creation);
    RenderPassHandle createRenderPass(const RenderPassCreation& creation);
    FramebufferHandle createFramebuffer(const FramebufferCreation& creation);
    DescriptorSetLayoutHandle createDescriptorSetLayout(const DescriptorSetLayoutCreation& creation);
    ProgramHandle createProgram(const ProgramCreation& creation);
    PipelineHandle createPipeline(const PipelineCreation& creation);

    void destroyTexture(TextureHandle handle);
    void destroyRenderPass(RenderPassHandle handle);
    void destroyFramebuffer(FramebufferHandle handle);
    void destroyProgram(ProgramHandle handle);
    void destroyPipeline(PipelineHandle handle);

    CommandBuffer* getCommandBuffer(uint32_t thread, bool profile = true);

    const vk::DispatchLoaderDynamic& vkTable() const noexcept {
        return _vkTable;
    }

    VkDevice vkDevice() noexcept {
        return _device;
    }

    static constexpr gerium_uint32_t MaxFrames = 2;

protected:
    VkInstance instance() const noexcept {
        return _instance;
    }

private:
    friend CommandBuffer;

    enum class ResourceType {
        Buffer,
        Texture,
        Sampler,
        RenderPass,
        Framebuffer,
        Program,
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
    void createSynchronizations();
    void createSwapchain(Application* application);
    void resizeSwapchain();

    void printValidationLayers();
    void printExtensions();
    void printPhysicalDevices();

    std::vector<uint32_t> compileGLSL(const char* code, size_t size, VkShaderStageFlagBits stage, const char* name);
    VkRenderPass vkCreateRenderPass(const RenderPassOutput& output, const char* name);
    void deleteResources(bool forceDelete = false);
    void setObjectName(VkObjectType type, uint64_t handle, gerium_utf8_t name);
    int getPhysicalDeviceScore(VkPhysicalDevice device);
    QueueFamilies getQueueFamilies(VkPhysicalDevice device);
    Swapchain getSwapchain();
    void frameCountersAdvance() noexcept;

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
    Application* _application{};
    gerium_uint16_t _appWidth{};
    gerium_uint16_t _appHeight{};
    ObjectPtr<Logger> _logger;

    vk::DispatchLoaderDynamic _vkTable;
    VkInstance _instance{};
    VkSurfaceKHR _surface{};
    VkPhysicalDevice _physicalDevice{};
    VkDevice _device{};
    QueueFamilies _queueFamilies{};
    VkQueue _queueGraphic{};
    VkQueue _queueCompute{};
    VkQueue _queuePresent{};
    VkQueue _queueTransfer{};
    VmaAllocator _vmaAllocator{};
    VkSemaphore _imageAvailableSemaphores[MaxFrames]{};
    VkSemaphore _renderFinishedSemaphores[MaxFrames]{};
    VkFence _inFlightFences[MaxFrames]{};
    VkSwapchainKHR _swapchain{};
    VkSurfaceFormatKHR _swapchainFormat{};
    VkExtent2D _swapchainExtent{};
    RenderPassHandle _swapchainRenderPass{ Undefined };
    std::vector<FramebufferHandle> _swapchainFramebuffers{};
    gerium_uint32_t _swapchainImageIndex{};
    gerium_uint32_t _currentFrame{};
    gerium_uint32_t _previousFrame{ MaxFrames - 1 };
    gerium_uint32_t _absoluteFrame{};

    TexturePool _textures;
    RenderPassPool _renderPasses;
    DescriptorSetLayoutPool _descriptorSetLayouts;
    ProgramPool _programs;
    PipelinePool _pipelines;
    FramebufferPool _framebuffers;

    CommandBufferManager _commandBufferManager{};
    std::queue<ResourceDeletion> _deletionQueue{};
    std::map<gerium_uint64_t, RenderPassHandle> _renderPassCache{};
    CommandBuffer* _queuedCommandBuffers[16]{};
    gerium_uint32_t _numQueuedCommandBuffers{};

    VkPhysicalDeviceProperties _deviceProperties{};
    VkPhysicalDeviceMemoryProperties _deviceMemProperties{};
    uint32_t _uboAlignment{};
    uint32_t _ssboAlignment{};
    bool _profilerSupported{};
    bool _profilerEnabled{};

    //// Test
    PipelineHandle _pipeline{};
};

} // namespace gerium::vulkan

#endif
