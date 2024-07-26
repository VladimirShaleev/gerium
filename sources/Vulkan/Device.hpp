#ifndef GERIUM_WINDOWS_VULKAN_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_DEVICE_HPP

#include "../Application.hpp"
#include "../Gerium.hpp"
#include "../Logger.hpp"
#include "../StringPool.hpp"
#include "CommandBuffer.hpp"
#include "Resources.hpp"
#include "Utils.hpp"
#include "VkProfiler.hpp"

namespace gerium::vulkan {

class Device : public Object {
public:
    virtual ~Device();

    void create(Application* application, gerium_uint32_t version, bool enableValidations);

    bool newFrame();
    void submit(CommandBuffer* commandBuffer);
    void present();

    BufferHandle createBuffer(const BufferCreation& creation);
    TextureHandle createTexture(const TextureCreation& creation);
    SamplerHandle createSampler(const SamplerCreation& creation);
    RenderPassHandle createRenderPass(const RenderPassCreation& creation);
    FramebufferHandle createFramebuffer(const FramebufferCreation& creation);
    DescriptorSetHandle createDescriptorSet(const DescriptorSetCreation& creation);
    DescriptorSetLayoutHandle createDescriptorSetLayout(const DescriptorSetLayoutCreation& creation);
    ProgramHandle createProgram(const ProgramCreation& creation, bool saveSpirv);
    PipelineHandle createPipeline(const PipelineCreation& creation);

    void destroyBuffer(BufferHandle handle);
    void destroyTexture(TextureHandle handle);
    void destroySampler(SamplerHandle handle);
    void destroyRenderPass(RenderPassHandle handle);
    void destroyFramebuffer(FramebufferHandle handle);
    void destroyDescriptorSet(DescriptorSetHandle handle);
    void destroyDescriptorSetLayout(DescriptorSetLayoutHandle handle);
    void destroyProgram(ProgramHandle handle);
    void destroyPipeline(PipelineHandle handle);

    void* mapBuffer(BufferHandle handle, uint32_t offset = 0, uint32_t size = 0);
    void unmapBuffer(BufferHandle handle);

    void bind(DescriptorSetHandle handle, uint16_t binding, Handle resource);
    void updateDescriptorSet(DescriptorSetHandle handle, DescriptorSetLayoutHandle layoutHandle);

    CommandBuffer* getCommandBuffer(uint32_t thread, bool profile = true);

    uint32_t totalMemoryUsed();

    const vk::DispatchLoaderDynamic& vkTable() const noexcept {
        return _vkTable;
    }

    VkDevice vkDevice() noexcept {
        return _device;
    }

    gerium_uint32_t previousFrame() const noexcept {
        return _previousFrame;
    }

    gerium_uint32_t currentFrame() const noexcept {
        return _currentFrame;
    }

    gerium_uint32_t absoluteFrame() const noexcept {
        return _absoluteFrame;
    }

    VkQueryPool vkQueryPool() noexcept {
        return _queryPool;
    }

    double gpuFrequency() const noexcept {
        return _gpuFrequency;
    }

    VkProfiler* profiler() noexcept {
        return _profiler.get();
    }

    RenderPassHandle getSwapchainPass() const noexcept {
        return _swapchainRenderPass;
    }

    FramebufferHandle getSwapchainFramebuffer() const noexcept {
        return _swapchainFramebuffers[_swapchainImageIndex];
    }

    const VkExtent2D& getSwapchainExtent() const noexcept {
        return _swapchainExtent;
    }

    // TextureHandle

    const RenderPassOutput& getRenderPassOutput(RenderPassHandle handle) const noexcept {
        return _renderPasses.access(handle)->output;
    }

    bool isProfilerEnable() const noexcept {
        return _profilerEnabled;
    }

    void setProfilerEnable(bool enable) noexcept {
        if (_profilerSupported) {
            _profilerEnabled = enable;
        }
    }

    static constexpr gerium_uint32_t MaxFrames = 2;

protected:
    VkInstance instance() const noexcept {
        return _instance;
    }

private:
    friend CommandBuffer;
    friend CommandBufferManager;

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
    void createProfiler(uint16_t gpuTimeQueriesPerFrame);
    void createDescriptorPool();
    void createVmaAllocator();
    void createDynamicBuffer();
    void createDefaultSampler();
    void createSynchronizations();
    void createSwapchain(Application* application);
    void createImGui(Application* application);
    void resizeSwapchain();

    void printValidationLayers();
    void printExtensions();
    void printPhysicalDevices();

    uint32_t fillWriteDescriptorSets(const DescriptorSetLayout& descriptorSetLayout,
                                     const DescriptorSet& descriptorSet,
                                     VkWriteDescriptorSet* descriptorWrite,
                                     VkDescriptorBufferInfo* bufferInfo,
                                     VkDescriptorImageInfo* imageInfo);
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
    std::vector<const char*> selectDeviceExtensions(VkPhysicalDevice device);
    VkPhysicalDevice selectPhysicalDevice();
    VkSurfaceFormatKHR selectSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR selectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes);
    VkExtent2D selectSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, Application* application);

    std::vector<const char*> checkValidationLayers(const std::vector<const char*>& layers);
    std::vector<const char*> checkExtensions(const std::vector<std::pair<const char*, bool>>& extensions);
    std::vector<const char*> checkDeviceExtensions(VkPhysicalDevice device,
                                                   const std::vector<std::pair<const char*, bool>>& extensions);
    bool checkPhysicalDeviceExtensions(VkPhysicalDevice device, const std::vector<const char*>& extensions);

    void debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData);

    static gerium_uint64_t calcPipelineHash(const PipelineCreation& creation) noexcept;

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                void* pUserData);

    static PFN_vkVoidFunction imguiLoaderFunc(const char* functionName, void* userData);

    virtual const char* onGetSurfaceExtension() const noexcept           = 0;
    virtual VkSurfaceKHR onCreateSurface(Application* application) const = 0;

    bool _enableValidations{};
    bool _enableDebugNames{};
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
    VkQueryPool _queryPool{};
    VkDescriptorPool _descriptorPool{};
    VkDescriptorPool _imguiPool{};
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
    uint32_t _dynamicBufferSize{};
    uint32_t _dynamicAllocatedSize{};
    BufferHandle _dynamicBuffer{ Undefined };
    uint8_t* _dynamicBufferMapped{};
    SamplerHandle _defaultSampler{ Undefined };

    BufferPool _buffers;
    TexturePool _textures;
    SamplerPool _samplers;
    RenderPassPool _renderPasses;
    DescriptorSetPool _descriptorSets;
    DescriptorSetLayoutPool _descriptorSetLayouts;
    ProgramPool _programs;
    PipelinePool _pipelines;
    FramebufferPool _framebuffers;

    CommandBufferManager _commandBufferManager{};
    std::queue<ResourceDeletion> _deletionQueue{};
    std::map<gerium_uint64_t, RenderPassHandle> _renderPassCache{};
    CommandBuffer* _queuedCommandBuffers[16]{};
    CommandBuffer* _frameCommandBuffer{};
    gerium_uint32_t _numQueuedCommandBuffers{};

    VkPhysicalDeviceProperties _deviceProperties{};
    VkPhysicalDeviceMemoryProperties _deviceMemProperties{};
    uint32_t _uboAlignment{};
    uint32_t _ssboAlignment{};
    bool _profilerSupported{};
    bool _profilerEnabled{};
    bool _deviceProperties2Supported{};
    bool _memoryBudgetSupported{};
    bool _imageAliasingSupported{};
    double _gpuFrequency{};
    ObjectPtr<VkProfiler> _profiler{};
    std::vector<VmaBudget> _vmaBudget{};
};

} // namespace gerium::vulkan

#endif
