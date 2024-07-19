#ifndef GERIUM_RENDERER_HPP
#define GERIUM_RENDERER_HPP

#include "Handles.hpp"
#include "ObjectPtr.hpp"
#include "Profiler.hpp"

struct _gerium_renderer : public gerium::Object {};

namespace gerium {

class FrameGraph;
class FrameGraphNode;

class Renderer : public _gerium_renderer {
public:
    Renderer() noexcept;

    void initialize(gerium_uint32_t version, bool debug);

    bool getProfilerEnable() const noexcept;
    void setProfilerEnable(bool enable) noexcept;

    BufferHandle createBuffer(const BufferCreation& creation);
    TextureHandle createTexture(const TextureCreation& creation);
    MaterialHandle createMaterial(const FrameGraph& frameGraph,
                                  gerium_utf8_t name,
                                  gerium_uint32_t pipelineCount,
                                  const gerium_pipeline_t* pipelines);
    DescriptorSetHandle createDescriptorSet();
    RenderPassHandle createRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node);
    FramebufferHandle createFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node);

    void destroyBuffer(BufferHandle handle) noexcept;
    void destroyTexture(TextureHandle handle) noexcept;
    void destroyMaterial(MaterialHandle handle) noexcept;
    void destroyDescriptorSet(DescriptorSetHandle handle) noexcept;
    void destroyRenderPass(RenderPassHandle handle) noexcept;
    void destroyFramebuffer(FramebufferHandle handle) noexcept;

    void bind(DescriptorSetHandle handle,
              gerium_uint16_t binding,
              const FrameGraph& frameGraph,
              gerium_utf8_t name) noexcept;

    bool newFrame();
    void render(FrameGraph& frameGraph);
    void present();

    Profiler* getProfiler() noexcept;
    void getSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept;

protected:
    virtual void onInitialize(gerium_uint32_t version, bool debug) = 0;

private:
    virtual bool onGetProfilerEnable() const noexcept      = 0;
    virtual void onSetProfilerEnable(bool enable) noexcept = 0;

    virtual BufferHandle onCreateBuffer(const BufferCreation& creation)                                     = 0;
    virtual TextureHandle onCreateTexture(const TextureCreation& creation)                                  = 0;
    virtual MaterialHandle onCreateMaterial(const FrameGraph& frameGraph,
                                            gerium_utf8_t name,
                                            gerium_uint32_t pipelineCount,
                                            const gerium_pipeline_t* pipelines)                             = 0;
    virtual DescriptorSetHandle onCreateDescriptorSet()                                                     = 0;
    virtual RenderPassHandle onCreateRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node)   = 0;
    virtual FramebufferHandle onCreateFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node) = 0;

    virtual void onDestroyBuffer(BufferHandle handle) noexcept               = 0;
    virtual void onDestroyTexture(TextureHandle handle) noexcept             = 0;
    virtual void onDestroyMaterial(MaterialHandle handle) noexcept           = 0;
    virtual void onDestroyDescriptorSet(DescriptorSetHandle handle) noexcept = 0;
    virtual void onDestroyRenderPass(RenderPassHandle handle) noexcept       = 0;
    virtual void onDestroyFramebuffer(FramebufferHandle handle) noexcept     = 0;

    virtual void onBind(DescriptorSetHandle handle,
                        gerium_uint16_t binding,
                        const FrameGraph& frameGraph,
                        gerium_utf8_t name) noexcept = 0;

    virtual bool onNewFrame()                     = 0;
    virtual void onRender(FrameGraph& frameGraph) = 0;
    virtual void onPresent()                      = 0;

    virtual Profiler* onGetProfiler() noexcept                                                      = 0;
    virtual void onGetSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept = 0;
};

} // namespace gerium

#endif
