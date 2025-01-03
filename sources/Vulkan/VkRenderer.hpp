#ifndef GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP

#include "../Application.hpp"
#include "../Logger.hpp"
#include "../Renderer.hpp"
#include "CommandBufferPool.hpp"
#include "Device.hpp"

namespace gerium::vulkan {

class VkRenderer : public Renderer {
public:
    VkRenderer(Application* application, ObjectPtr<Device>&& device) noexcept;
    ~VkRenderer() override;

    PipelineHandle getPipeline(TechniqueHandle handle) const noexcept;

protected:
    void onInitialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) override;

    Application* application() noexcept;

private:
    struct LoadRequest {
        gerium_uint32_t dataSize{};
        gerium_cdata_t data{};
        gerium_uint8_t mip{};
        bool generateMips{};
        TextureHandle texture{ Undefined };
        gerium_texture_loaded_func_t callback{};
        gerium_data_t userData{};
    };

    void createTransferBuffer();
    void sendTextureToGraphic();
    bool isResourceEnabled(FrameGraph& frameGraph, const FrameGraphResource* resource) const noexcept;

    gerium_feature_flags_t onGetEnabledFeatures() const noexcept override;
    TextureCompressionFlags onGetTextureComperssion() const noexcept override;

    bool onGetProfilerEnable() const noexcept override;
    void onSetProfilerEnable(bool enable) noexcept override;

    bool onIsSupportedFormat(gerium_format_t format) noexcept override;
    void onGetTextureInfo(TextureHandle handle, gerium_texture_info_t& info) noexcept override;

    BufferHandle onCreateBuffer(const BufferCreation& creation) override;
    TextureHandle onCreateTexture(const TextureCreation& creation) override;
    TextureHandle onCreateTextureView(const TextureViewCreation& creation) override;
    TechniqueHandle onCreateTechnique(const FrameGraph& frameGraph,
                                      gerium_utf8_t name,
                                      gerium_uint32_t pipelineCount,
                                      const gerium_pipeline_t* pipelines) override;
    DescriptorSetHandle onCreateDescriptorSet(bool global) override;
    RenderPassHandle onCreateRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) override;
    FramebufferHandle onCreateFramebuffer(const FrameGraph& frameGraph,
                                          const FrameGraphNode* node,
                                          gerium_uint32_t textureIndex) override;

    void onAsyncUploadTextureData(TextureHandle handle,
                                  gerium_uint8_t mip,
                                  bool generateMips,
                                  gerium_uint32_t textureDataSize,
                                  gerium_cdata_t textureData,
                                  gerium_texture_loaded_func_t callback,
                                  gerium_data_t data) override;

    void onTextureSampler(TextureHandle handle,
                          gerium_filter_t minFilter,
                          gerium_filter_t magFilter,
                          gerium_filter_t mipFilter,
                          gerium_address_mode_t addressModeU,
                          gerium_address_mode_t addressModeV,
                          gerium_address_mode_t addressModeW,
                          gerium_reduction_mode_t reductionMode) override;

    BufferHandle onGetBuffer(gerium_utf8_t resource) override;
    TextureHandle onGetTexture(gerium_utf8_t resource, bool fromPreviousFrame) override;

    void onDestroyBuffer(BufferHandle handle) noexcept override;
    void onDestroyTexture(TextureHandle handle) noexcept override;
    void onDestroyTechnique(TechniqueHandle handle) noexcept override;
    void onDestroyDescriptorSet(DescriptorSetHandle handle) noexcept override;
    void onDestroyRenderPass(RenderPassHandle handle) noexcept override;
    void onDestroyFramebuffer(FramebufferHandle handle) noexcept override;

    void onBind(DescriptorSetHandle handle, gerium_uint16_t binding, BufferHandle buffer) noexcept override;
    void onBind(DescriptorSetHandle handle,
                gerium_uint16_t binding,
                gerium_uint16_t element,
                TextureHandle texture) noexcept override;
    void onBind(DescriptorSetHandle handle,
                gerium_uint16_t binding,
                gerium_utf8_t resourceInput,
                bool fromPreviousFrame) noexcept override;

    gerium_data_t onMapBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size) noexcept override;
    void onUnmapBuffer(BufferHandle handle) noexcept override;

    bool onNewFrame() override;
    void onRender(FrameGraph& frameGraph) override;
    void onPresent() override;

    FfxInterface onCreateFfxInterface(gerium_uint32_t maxContexts) override;
    void onWaitFfxJobs() const noexcept override;
    void onDestroyFfxInterface(FfxInterface* ffxInterface) noexcept override;
    FfxResource onGetFfxBuffer(BufferHandle handle) const noexcept override;
    FfxResource onGetFfxTexture(TextureHandle handle) const noexcept override;

    Profiler* onGetProfiler() noexcept override;
    void onGetSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept override;

    void loadThread() noexcept;

    ObjectPtr<Application> _application;
    ObjectPtr<Device> _device;
    bool _isSupportedTransferQueue;
    gerium_uint16_t _width;
    gerium_uint16_t _height;
    gerium_utf8_t _currentRenderPassName;
    TechniquePool _techniques;
    gerium_uint32_t _transferMaxTasks;
    BufferHandle _transferBuffer;
    size_t _transferBufferOffset;
    CommandBufferPool _transferCommandPool;
    std::thread _loadTread;
    marl::Event _loadEvent;
    marl::Event _loadThreadEnd;
    marl::mutex _loadRequestsMutex;
    marl::mutex _transferToGraphicMutex;
    std::queue<LoadRequest> _loadRequests;
    std::queue<LoadRequest> _transferToGraphic;
    std::queue<LoadRequest> _finishedRequests;
    gerium_uint32_t _prevFrame;
    gerium_uint32_t _frame;
};

} // namespace gerium::vulkan

#endif
