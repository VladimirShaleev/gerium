#ifndef GERIUM_RENDERER_HPP
#define GERIUM_RENDERER_HPP

#include "Handles.hpp"
#include "Logger.hpp"
#include "Profiler.hpp"

struct _gerium_renderer : public gerium::Object {};

namespace gerium {

class FrameGraph;
class FrameGraphNode;

class Renderer : public _gerium_renderer {
public:
    Renderer() noexcept;
    ~Renderer() override;

    void initialize(gerium_feature_flags_t features, const gerium_renderer_options_t* options);

    gerium_feature_flags_t getEnabledFeatures() const noexcept;
    TextureCompressionFlags getTextureComperssion() const noexcept;

    bool getProfilerEnable() const noexcept;
    void setProfilerEnable(bool enable) noexcept;

    gerium_uint32_t getFramesInFlight() const noexcept;

    bool isSupportedFormat(gerium_format_t format) noexcept;
    void getTextureInfo(TextureHandle handle, gerium_texture_info_t& info) noexcept;

    bool isAutoRotate() const noexcept;
    int getRotate() const noexcept;

    BufferHandle createBuffer(const BufferCreation& creation);
    TextureHandle createTexture(const TextureCreation& creation);
    TextureHandle createTextureView(const TextureViewCreation& creation);
    TechniqueHandle createTechnique(const FrameGraph& frameGraph,
                                    gerium_utf8_t name,
                                    gerium_uint32_t pipelineCount,
                                    const gerium_pipeline_t* pipelines);
    DescriptorSetHandle createDescriptorSet(bool global);
    RenderPassHandle createRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node);
    FramebufferHandle createFramebuffer(const FrameGraph& frameGraph,
                                        const FrameGraphNode* node,
                                        gerium_uint32_t textureIndex);

    TextureHandle asyncLoadTexture(gerium_utf8_t filename, gerium_texture_loaded_func_t callback, gerium_data_t data);

    void asyncUploadTextureData(TextureHandle handle,
                                gerium_uint8_t mip,
                                bool generateMips,
                                gerium_uint32_t textureDataSize,
                                gerium_cdata_t textureData,
                                gerium_texture_loaded_func_t callback,
                                gerium_data_t data);

    void textureSampler(TextureHandle handle,
                        gerium_filter_t minFilter,
                        gerium_filter_t magFilter,
                        gerium_filter_t mipFilter,
                        gerium_address_mode_t addressModeU,
                        gerium_address_mode_t addressModeV,
                        gerium_address_mode_t addressModeW,
                        gerium_reduction_mode_t reductionMode);
    BufferHandle getBuffer(gerium_utf8_t resource);
    TextureHandle getTexture(gerium_utf8_t resource, bool fromPreviousFrame);

    void destroyBuffer(BufferHandle handle) noexcept;
    void destroyTexture(TextureHandle handle) noexcept;
    void destroyTechnique(TechniqueHandle handle) noexcept;
    void destroyDescriptorSet(DescriptorSetHandle handle) noexcept;
    void destroyRenderPass(RenderPassHandle handle) noexcept;
    void destroyFramebuffer(FramebufferHandle handle) noexcept;

    void bind(DescriptorSetHandle handle, gerium_uint16_t binding, BufferHandle buffer) noexcept;
    void bind(DescriptorSetHandle handle,
              gerium_uint16_t binding,
              gerium_uint16_t element,
              TextureHandle texture) noexcept;
    void bind(DescriptorSetHandle handle,
              gerium_uint16_t binding,
              gerium_utf8_t resourceInput,
              bool fromPreviousFrame) noexcept;

    gerium_data_t mapBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size) noexcept;
    void unmapBuffer(BufferHandle handle) noexcept;

    bool newFrame();
    void render(FrameGraph& frameGraph);
    void present();

    FfxInterface createFfxInterface(gerium_uint32_t maxContexts);
    void destroyFfxInterface(FfxInterface* ffxInterface) noexcept;
    void waitFfxJobs() const noexcept;
    FfxResource getFfxBuffer(BufferHandle handle) const noexcept;
    FfxResource getFfxTexture(TextureHandle handle) const noexcept;

    Profiler* getProfiler() noexcept;
    void getSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept;
    gerium_format_t getSwapchainFormat() const noexcept;

protected:
    virtual void onInitialize(gerium_feature_flags_t features, const gerium_renderer_options_t& options) = 0;

    void closeLoadThread();

private:
    struct TaskMip {
        gerium_cdata_t imageData;
        gerium_uint32_t imageSize;
        gerium_uint8_t imageMip;
    };

    struct Task {
        Renderer* renderer;
        TextureHandle texture;
        ObjectPtr<File> file;
        gerium_cdata_t data;
        ktxTexture2* ktxTexture;
        gerium_uint8_t imageGenerateMips;
        std::queue<TaskMip> mips;
        gerium_texture_loaded_func_t callback;
        gerium_data_t userData;
    };

    virtual gerium_feature_flags_t onGetEnabledFeatures() const noexcept     = 0;
    virtual TextureCompressionFlags onGetTextureComperssion() const noexcept = 0;

    virtual bool onGetProfilerEnable() const noexcept      = 0;
    virtual void onSetProfilerEnable(bool enable) noexcept = 0;

    virtual gerium_uint32_t onGetFramesInFlight() const noexcept = 0;

    virtual bool onIsSupportedFormat(gerium_format_t format) noexcept                         = 0;
    virtual void onGetTextureInfo(TextureHandle handle, gerium_texture_info_t& info) noexcept = 0;

    virtual bool onIsAutoRotate() const noexcept;
    virtual int onGetRotate() const noexcept;

    virtual BufferHandle onCreateBuffer(const BufferCreation& creation)                                   = 0;
    virtual TextureHandle onCreateTexture(const TextureCreation& creation)                                = 0;
    virtual TextureHandle onCreateTextureView(const TextureViewCreation& creation)                        = 0;
    virtual TechniqueHandle onCreateTechnique(const FrameGraph& frameGraph,
                                              gerium_utf8_t name,
                                              gerium_uint32_t pipelineCount,
                                              const gerium_pipeline_t* pipelines)                         = 0;
    virtual DescriptorSetHandle onCreateDescriptorSet(bool global)                                        = 0;
    virtual RenderPassHandle onCreateRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) = 0;
    virtual FramebufferHandle onCreateFramebuffer(const FrameGraph& frameGraph,
                                                  const FrameGraphNode* node,
                                                  gerium_uint32_t textureIndex)                           = 0;

    virtual void onAsyncUploadTextureData(TextureHandle handle,
                                          gerium_uint8_t mip,
                                          bool generateMips,
                                          gerium_uint32_t textureDataSize,
                                          gerium_cdata_t textureData,
                                          gerium_texture_loaded_func_t callback,
                                          gerium_data_t data) = 0;

    virtual void onTextureSampler(TextureHandle handle,
                                  gerium_filter_t minFilter,
                                  gerium_filter_t magFilter,
                                  gerium_filter_t mipFilter,
                                  gerium_address_mode_t addressModeU,
                                  gerium_address_mode_t addressModeV,
                                  gerium_address_mode_t addressModeW,
                                  gerium_reduction_mode_t reductionMode) = 0;

    virtual BufferHandle onGetBuffer(gerium_utf8_t resource)                           = 0;
    virtual TextureHandle onGetTexture(gerium_utf8_t resource, bool fromPreviousFrame) = 0;

    virtual void onDestroyBuffer(BufferHandle handle) noexcept               = 0;
    virtual void onDestroyTexture(TextureHandle handle) noexcept             = 0;
    virtual void onDestroyTechnique(TechniqueHandle handle) noexcept         = 0;
    virtual void onDestroyDescriptorSet(DescriptorSetHandle handle) noexcept = 0;
    virtual void onDestroyRenderPass(RenderPassHandle handle) noexcept       = 0;
    virtual void onDestroyFramebuffer(FramebufferHandle handle) noexcept     = 0;

    virtual void onBind(DescriptorSetHandle handle, gerium_uint16_t binding, BufferHandle buffer) noexcept = 0;

    virtual void onBind(DescriptorSetHandle handle,
                        gerium_uint16_t binding,
                        gerium_uint16_t element,
                        TextureHandle texture) noexcept = 0;

    virtual void onBind(DescriptorSetHandle handle,
                        gerium_uint16_t binding,
                        gerium_utf8_t resourceInput,
                        bool fromPreviousFrame) noexcept = 0;

    virtual gerium_data_t onMapBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size) noexcept = 0;
    virtual void onUnmapBuffer(BufferHandle handle) noexcept                                                      = 0;

    virtual bool onNewFrame()                     = 0;
    virtual void onRender(FrameGraph& frameGraph) = 0;
    virtual void onPresent()                      = 0;

    virtual FfxInterface onCreateFfxInterface(gerium_uint32_t maxContexts)   = 0;
    virtual void onWaitFfxJobs() const noexcept                              = 0;
    virtual void onDestroyFfxInterface(FfxInterface* ffxInterface) noexcept  = 0;
    virtual FfxResource onGetFfxBuffer(BufferHandle handle) const noexcept   = 0;
    virtual FfxResource onGetFfxTexture(TextureHandle handle) const noexcept = 0;

    virtual Profiler* onGetProfiler() noexcept                                                      = 0;
    virtual void onGetSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept = 0;
    virtual gerium_format_t onGetSwapchainFormat() const noexcept                                   = 0;

    Task* createLoadTask(ObjectPtr<File> file, const std::string& name);
    Task* createLoadTaskKtx2(ObjectPtr<File> file, const std::string& name);

    void loadThread() noexcept;

    static KTX_error_code loadMips(int miplevel,
                                   int face,
                                   int width,
                                   int height,
                                   int depth,
                                   ktx_uint64_t faceLodSize,
                                   void* pixels,
                                   void* userdata);

    ObjectPtr<Logger> _logger;
    std::thread _loadThread;
    marl::Event _shutdownSignal;
    marl::Event _waitTaskSignal;
    marl::mutex _loadRequestsMutex;
    std::queue<Task*> _tasks;
};

} // namespace gerium

#endif
