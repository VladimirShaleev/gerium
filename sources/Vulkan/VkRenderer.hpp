#ifndef GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP
#define GERIUM_WINDOWS_VULKAN_VK_RENDERER_HPP

#include "../Application.hpp"
#include "../Logger.hpp"
#include "../Renderer.hpp"
#include "Device.hpp"

namespace gerium::vulkan {

class VkRenderer : public Renderer {
public:
    VkRenderer(Application* application, std::unique_ptr<Device>&& device) noexcept;

protected:
    void onInitialize(gerium_uint32_t version, bool debug) override;

    Application* application() noexcept;

private:
    BufferHandle onCreateBuffer(const gerium_buffer_creation_t& creation) override;
    TextureHandle onCreateTexture(const TextureCreation& creation) override;
    MaterialHandle onCreateMaterial(const FrameGraph& frameGraph,
                                    gerium_utf8_t name,
                                    gerium_uint32_t pipelineCount,
                                    const gerium_pipeline_t* pipelines) override;
    RenderPassHandle onCreateRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) override;
    FramebufferHandle onCreateFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node) override;

    void onDestroyTexture(TextureHandle handle) noexcept override;

    bool onNewFrame() override;
    void onRender(const FrameGraph& frameGraph) override;
    void onPresent() override;

    Profiler* onGetProfiler() noexcept override;

    ObjectPtr<Application> _application;
    std::unique_ptr<Device> _device;
};

} // namespace gerium::vulkan

#endif
