#ifndef GERIUM_WINDOWS_VULKAN_DEVICE_HPP
#define GERIUM_WINDOWS_VULKAN_DEVICE_HPP

#include "../Application.hpp"
#include "../Gerium.hpp"
#include "../Logger.hpp"
#include "../StringPool.hpp"
#include "CommandBufferPool.hpp"
#include "Resources.hpp"
#include "Utils.hpp"
#include "VkProfiler.hpp"

namespace gerium::vulkan {

class Device : public Object {
public:
    virtual ~Device();

    void create(Application* application,
                gerium_feature_flags_t features,
                gerium_uint32_t version,
                bool enableValidations);

    bool newFrame();
    void submit(CommandBuffer* commandBuffer);
    void present();

    BufferHandle createBuffer(const BufferCreation& creation);
    TextureHandle createTexture(const TextureCreation& creation);
    TextureHandle createTextureView(const TextureViewCreation& creation);
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

    void finishLoadTexture(TextureHandle handle, uint8_t mip);
    void showViewMips(TextureHandle handle);

    void bind(DescriptorSetHandle handle,
              gerium_uint16_t binding,
              gerium_uint16_t element,
              Handle resource,
              bool dynamic                = false,
              gerium_utf8_t resourceInput = nullptr,
              bool fromPreviousFrame      = false);
    VkDescriptorSet updateDescriptorSet(DescriptorSetHandle handle,
                                        DescriptorSetLayoutHandle layoutHandle,
                                        FrameGraph* frameGraph);

    CommandBuffer* getPrimaryCommandBuffer(bool profile = true);
    CommandBuffer* getSecondaryCommandBuffer(gerium_uint32_t thread,
                                             RenderPassHandle renderPass,
                                             FramebufferHandle framebuffer);

    SamplerHandle getTextureSampler(TextureHandle texture) const noexcept;
    void linkTextureSampler(TextureHandle texture, SamplerHandle sampler) noexcept;

    FfxInterface createFfxInterface(gerium_uint32_t maxContexts);
    void destroyFfxInterface(FfxInterface* ffxInterface);
    void waitFfxJobs() const noexcept;

    void clearInputResources();
    void addInputResource(const FrameGraphResource* resource, Handle handle, bool fromPreviousFrame);
    Handle findInputResource(gerium_utf8_t resource, bool fromPreviousFrame) const noexcept;
    static gerium_uint64_t calcKeyInputResource(gerium_utf8_t resource, bool fromPreviousFrame) noexcept;

    bool isSupportedFormat(gerium_format_t format) noexcept;

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

    bool bindlessSupported() const noexcept {
        return _bindlessSupported;
    }

    bool meshShaderSupported() const noexcept {
        return _meshShaderSupported;
    }

    TextureCompressionFlags compressions() const noexcept {
        return _compressions;
    }

    bool isProfilerEnable() const noexcept {
        return _profilerEnabled;
    }

    void setProfilerEnable(bool enable) noexcept {
        if (_profilerSupported) {
            _profilerEnabled = enable;
        }
    }

    // uint32_t getTransferFamily() const noexcept {
    //     return _queueFamilies.transfer.value().index;
    // }

    // uint32_t getGraphicFamily() const noexcept {
    //     return _queueFamilies.graphic.value().index;
    // }

    void getTextureInfo(TextureHandle handle, gerium_texture_info_t& info) noexcept {
        const auto texture = _textures.access(handle);

        info.width   = texture->width;
        info.height  = texture->height;
        info.depth   = texture->depth;
        info.mipmaps = texture->mipLevels;
        info.layers  = texture->layers;
        info.format  = toGeriumFormat(texture->vkFormat);
        info.type    = texture->type;
        info.name    = texture->name;
    }

    bool isBufferDynamic(BufferHandle handle) const noexcept {
        return _buffers.access(handle)->parent != Undefined;
    }

    bool isSupportedTransferQueue() const noexcept {
        return !_queueFamilies.transferIsGraphic;
    }

    FfxResource ffxBuffer(BufferHandle handle) const noexcept {
        auto buffer = _buffers.access(handle);

        FfxResourceDescription resourceDescription{};
        resourceDescription.type  = FFX_RESOURCE_TYPE_BUFFER;
        resourceDescription.size  = buffer->size;
        resourceDescription.flags = FFX_RESOURCE_FLAGS_NONE;
        resourceDescription.usage = FFX_RESOURCE_USAGE_READ_ONLY;
        // resourceDescription.stride = 16;
        if (buffer->vkUsageFlags & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) {
            resourceDescription.usage = (FfxResourceUsage) (resourceDescription.usage | FFX_RESOURCE_USAGE_UAV);
        }

        FfxResource resource{};
        resource.resource    = reinterpret_cast<void*>(buffer->vkBuffer);
        resource.description = resourceDescription;
        resource.state       = FFX_RESOURCE_STATE_UNORDERED_ACCESS; // FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ;

        return resource;
    }

    FfxResource ffxTexture(TextureHandle handle) const noexcept {
        auto texture = _textures.access(handle);

        if (texture->loadedMips == 0) {
            texture = _textures.access(_defaultTexture);
        }

        FfxResourceDescription resourceDescription{};

        switch (texture->type) {
            case GERIUM_TEXTURE_TYPE_1D:
                resourceDescription.type  = FFX_RESOURCE_TYPE_TEXTURE1D;
                resourceDescription.depth = texture->layers;
                break;
            case GERIUM_TEXTURE_TYPE_2D:
                resourceDescription.type  = FFX_RESOURCE_TYPE_TEXTURE2D;
                resourceDescription.depth = texture->layers;
                break;
            case GERIUM_TEXTURE_TYPE_3D:
                resourceDescription.type  = FFX_RESOURCE_TYPE_TEXTURE3D;
                resourceDescription.depth = texture->depth;
                break;
            case GERIUM_TEXTURE_TYPE_CUBE:
                resourceDescription.type  = FFX_RESOURCE_TYPE_TEXTURE_CUBE;
                resourceDescription.depth = texture->layers;
                break;
        }

        resourceDescription.usage = FFX_RESOURCE_USAGE_READ_ONLY;

        if (hasDepth(texture->vkFormat)) {
            resourceDescription.usage = (FfxResourceUsage) (resourceDescription.usage | FFX_RESOURCE_USAGE_DEPTHTARGET);
        }
        if (hasStencil(texture->vkFormat)) {
            resourceDescription.usage =
                (FfxResourceUsage) (resourceDescription.usage | FFX_RESOURCE_USAGE_STENCILTARGET);
        }
        if ((texture->flags & TextureFlags::Compute) == TextureFlags::Compute) {
            resourceDescription.usage = (FfxResourceUsage) (resourceDescription.usage | FFX_RESOURCE_USAGE_UAV);
        }

        resourceDescription.width    = texture->width;
        resourceDescription.height   = texture->height;
        resourceDescription.mipCount = texture->mipLevels;
        resourceDescription.format   = ffxGetSurfaceFormatVK(texture->vkFormat);

        FfxResource resource{};
        resource.resource    = reinterpret_cast<void*>(texture->vkImage);
        resource.description = resourceDescription;

        constexpr int FFX_RESOURCE_STATE_DEPTH_STENCIL_READ_ONLY = (1 << 9);
        switch (texture->states[0]) {
            case ResourceState::ShaderResource:
                resource.state = FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ;
                break;
            case ResourceState::DepthRead:
                resource.state = (FfxResourceStates) FFX_RESOURCE_STATE_DEPTH_STENCIL_READ_ONLY;
                break;
            default:
                resource.state = FFX_RESOURCE_STATE_UNORDERED_ACCESS;
                break;
        }

        return resource;
    }

protected:
    VkInstance instance() const noexcept {
        return _instance;
    }

private:
    friend CommandBuffer;
    friend CommandBufferPool;

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
        bool transferIsGraphic;

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
    void createDevice(gerium_uint32_t threadCount, gerium_feature_flags_t featureFlags);
    void createProfiler(uint16_t gpuTimeQueriesPerFrame);
    void createDescriptorPools();
    void createVmaAllocator();
    void createDynamicBuffers();
    void createDefaultSampler();
    void createDefaultTexture();
    void createSynchronizations();
    void createSwapchain(Application* application);
    void createImGui(Application* application);
    void createFidelityFX();
    void resizeSwapchain();

    void printValidationLayers();
    void printExtensions();
    void printPhysicalDevices();

    std::tuple<uint32_t, bool> fillWriteDescriptorSets(const DescriptorSetLayout& descriptorSetLayout,
                                                       const DescriptorSet& descriptorSet,
                                                       VkWriteDescriptorSet* descriptorWrite,
                                                       VkDescriptorBufferInfo* bufferInfo,
                                                       VkDescriptorImageInfo* imageInfo);
    std::vector<uint32_t> compile(const char* code,
                                  size_t size,
                                  gerium_shader_languge_t lang,
                                  VkShaderStageFlagBits stage,
                                  const char* name,
                                  gerium_uint32_t numMacros,
                                  const gerium_macro_definition_t* macros);
    VkRenderPass vkCreateRenderPass(const RenderPassOutput& output, const char* name);
    void vkCreateImageView(const TextureViewCreation& creation, TextureHandle handle);
    void deleteResources(bool forceDelete = false);
    void setObjectName(VkObjectType type, uint64_t handle, gerium_utf8_t name);
    int getPhysicalDeviceScore(VkPhysicalDevice device);
    QueueFamilies getQueueFamilies(VkPhysicalDevice device);
    Swapchain getSwapchain();
    void frameCountersAdvance() noexcept;
    void uploadTextureData(TextureHandle handle, gerium_cdata_t data);

    std::vector<const char*> selectValidationLayers();
    std::vector<const char*> selectExtensions();
    std::vector<const char*> selectDeviceExtensions(VkPhysicalDevice device, bool meshShader);
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
    static gerium_uint64_t calcSamplerHash(const SamplerCreation& creation) noexcept;
    static gerium_uint32_t calcBindingKey(gerium_uint16_t binding, gerium_uint16_t element) noexcept;

    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                void* pUserData);

    static PFN_vkVoidFunction imguiLoaderFunc(const char* functionName, void* userData);

    virtual VkInstanceCreateFlags onGetCreateInfoFlags() const noexcept;
    virtual const void* onGetNextCreateInfo(void* pNext) noexcept;
    virtual std::vector<const char*> onGetInstanceExtensions() const noexcept;
    virtual std::vector<const char*> onGetDeviceExtensions() const noexcept;
    virtual bool onNeedPostAcquireResize() const noexcept;
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
    marl::mutex _descriptorPoolMutex{};
    VkDescriptorPool _globalDescriptorPool{};
    VkDescriptorPool _descriptorPools[kMaxFrames]{};
    VkDescriptorPool _imguiPool{};
    VmaAllocator _vmaAllocator{};
    VkSemaphore _imageAvailableSemaphores[kMaxFrames]{};
    VkSemaphore _renderFinishedSemaphores[kMaxFrames]{};
    VkFence _inFlightFences[kMaxFrames]{};
    VkSwapchainKHR _swapchain{};
    VkSurfaceFormatKHR _swapchainFormat{};
    VkExtent2D _swapchainExtent{};
    RenderPassHandle _swapchainRenderPass{ Undefined };
    std::vector<FramebufferHandle> _swapchainFramebuffers{};
    std::set<TextureHandle> _swapchainImages{};
    gerium_uint32_t _swapchainImageIndex{};
    gerium_uint32_t _currentFrame{};
    gerium_uint32_t _previousFrame{ kMaxFrames - 1 };
    gerium_uint32_t _absoluteFrame{};
    uint32_t _dynamicUBOSize{};
    uint32_t _dynamicSSBOSize{};
    uint32_t _dynamicUBOAllocatedSize{};
    uint32_t _dynamicSSBOAllocatedSize{};
    BufferHandle _dynamicUBO{ Undefined };
    BufferHandle _dynamicSSBO{ Undefined };
    uint8_t* _dynamicUBOMapped{};
    uint8_t* _dynamicSSBOMapped{};
    SamplerHandle _defaultSampler{ Undefined };
    TextureHandle _defaultTexture{ Undefined };
    VkWriteDescriptorSet _descriptorWrite[kBindlessPoolElements]{};
    VkDescriptorBufferInfo _bufferInfo[kBindlessPoolElements]{};
    VkDescriptorImageInfo _imageInfo[kBindlessPoolElements]{};

    BufferPool _buffers;
    TexturePool _textures;
    SamplerPool _samplers;
    RenderPassPool _renderPasses;
    DescriptorSetPool _descriptorSets;
    DescriptorSetLayoutPool _descriptorSetLayouts;
    ProgramPool _programs;
    PipelinePool _pipelines;
    FramebufferPool _framebuffers;

    CommandBufferPool _commandBufferPool{};
    std::queue<ResourceDeletion> _deletionQueue{};
    std::map<gerium_uint64_t, RenderPassHandle> _renderPassCache{};
    CommandBuffer* _queuedCommandBuffers[16]{};
    CommandBuffer* _frameCommandBuffer{};
    gerium_uint32_t _numQueuedCommandBuffers{};
    std::map<gerium_uint64_t, SamplerHandle> _samplerCache{};
    VkDescriptorSet _freeDescriptorSetQueue[64]{};
    VkDescriptorSet _freeDescriptorSetQueue2[64]{};
    gerium_uint32_t _numFreeDescriptorSetQueue{};
    gerium_uint32_t _numFreeDescriptorSetQueue2{};
    std::vector<std::pair<gerium_uint32_t, VkImageView>> _unusedImageViews{};
    std::map<gerium_uint64_t, Handle> _currentInputResources{};

    VkPhysicalDeviceProperties _deviceProperties{};
    VkPhysicalDeviceMemoryProperties _deviceMemProperties{};
    uint32_t _alignment{};
    bool _profilerSupported{};
    bool _profilerEnabled{};
    bool _memoryBudgetSupported{};
    bool _bindlessSupported{};
    bool _fidelityFXSupported{};
    bool _meshShaderSupported{};
    bool _8BitStorageSupported{};
    bool _16BitStorageSupported{};
    TextureCompressionFlags _compressions{};
    double _gpuFrequency{};
    ObjectPtr<VkProfiler> _profiler{};
    std::vector<VmaBudget> _vmaBudget{};

#ifdef GERIUM_FIDELITY_FX
    VkDeviceContext _ffxDeviceContext{};
#endif
};

} // namespace gerium::vulkan

#endif
