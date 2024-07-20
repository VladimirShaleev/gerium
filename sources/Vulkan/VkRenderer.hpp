#ifndef GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP

#include "../Application.hpp"
#include "../Logger.hpp"
#include "../Renderer.hpp"
#include "Device.hpp"

namespace gerium::vulkan {

class VkRenderer : public Renderer {
public:
    VkRenderer(Application* application, ObjectPtr<Device>&& device) noexcept;

    PipelineHandle getPipeline(MaterialHandle handle) const noexcept;

protected:
    void onInitialize(gerium_uint32_t version, bool debug) override;

    Application* application() noexcept;

private:
    bool onGetProfilerEnable() const noexcept override;
    void onSetProfilerEnable(bool enable) noexcept override;

    BufferHandle onCreateBuffer(const BufferCreation& creation) override;
    TextureHandle onCreateTexture(const TextureCreation& creation) override;
    MaterialHandle onCreateMaterial(const FrameGraph& frameGraph,
                                    gerium_utf8_t name,
                                    gerium_uint32_t pipelineCount,
                                    const gerium_pipeline_t* pipelines) override;
    DescriptorSetHandle onCreateDescriptorSet() override;
    RenderPassHandle onCreateRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) override;
    FramebufferHandle onCreateFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node) override;

    void onDestroyBuffer(BufferHandle handle) noexcept override;
    void onDestroyTexture(TextureHandle handle) noexcept override;
    void onDestroyMaterial(MaterialHandle handle) noexcept override;
    void onDestroyDescriptorSet(DescriptorSetHandle handle) noexcept override;
    void onDestroyRenderPass(RenderPassHandle handle) noexcept override;
    void onDestroyFramebuffer(FramebufferHandle handle) noexcept override;

    void onBind(DescriptorSetHandle handle, gerium_uint16_t binding, BufferHandle buffer) noexcept override;
    void onBind(DescriptorSetHandle handle,
                gerium_uint16_t binding,
                const FrameGraph& frameGraph,
                gerium_utf8_t name) noexcept override;

    gerium_data_t onMapBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size) noexcept override;
    void onUnmapBuffer(BufferHandle handle) noexcept override;

    bool onNewFrame() override;
    void onRender(FrameGraph& frameGraph) override;
    void onPresent() override;

    Profiler* onGetProfiler() noexcept override;
    void onGetSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept override;

    ObjectPtr<Application> _application;
    ObjectPtr<Device> _device;
    gerium_uint16_t _width;
    gerium_uint16_t _height;
    gerium_utf8_t _currentRenderPass;

    MaterialPool _materials;
};

} // namespace gerium::vulkan

#endif
