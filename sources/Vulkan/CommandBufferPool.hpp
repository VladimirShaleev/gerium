#ifndef GERIUM_WINDOWS_VULKAN_COMMAND_BUFFER_POOL_HPP
#define GERIUM_WINDOWS_VULKAN_COMMAND_BUFFER_POOL_HPP

#include "../CommandBuffer.hpp"
#include "../FrameGraph.hpp"
#include "Resources.hpp"

namespace gerium::vulkan {

class Device;

class CommandBuffer final : public gerium::CommandBuffer {
public:
    CommandBuffer() = default;
    CommandBuffer(Device& device, VkCommandBuffer commandBuffer);

    void addImageBarrier(TextureHandle handle,
                         ResourceState oldState,
                         ResourceState newState,
                         gerium_uint32_t mipLevel,
                         gerium_uint32_t mipCount);
    void clearColor(gerium_uint32_t index,
                    gerium_float32_t red,
                    gerium_float32_t green,
                    gerium_float32_t blue,
                    gerium_float32_t alpha) noexcept;
    void clearDepthStencil(gerium_float32_t depth, gerium_uint32_t value) noexcept;
    void bindPass(RenderPassHandle renderPass, FramebufferHandle framebuffer);
    void bindPipeline(PipelineHandle pipeline);
    void copyBuffer(BufferHandle src, BufferHandle dst);
    void pushMarker(gerium_utf8_t name);
    void popMarker();
    void submit(QueueType queue);
    void execute(gerium_uint32_t numCommandBuffers, CommandBuffer* commandBuffers[]);

    void begin(RenderPassHandle renderPass = Undefined, FramebufferHandle framebuffer = Undefined);
    void end();
    void endCurrentRenderPass();

    VkCommandBuffer vkCommandBuffer() const noexcept;

    void setFramebufferHeight(gerium_uint16_t framebufferHeight) noexcept;
    void setFrameGraph(FrameGraph* frameGraph) noexcept;

    bool isRecording() const noexcept;

private:
    void onSetViewport(gerium_uint16_t x,
                       gerium_uint16_t y,
                       gerium_uint16_t width,
                       gerium_uint16_t height,
                       gerium_float32_t minDepth,
                       gerium_float32_t maxDepth) noexcept override;
    void onSetScissor(gerium_uint16_t x,
                      gerium_uint16_t y,
                      gerium_uint16_t width,
                      gerium_uint16_t height) noexcept override;
    void onBindTechnique(TechniqueHandle handle) noexcept override;
    void onBindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept override;
    void onBindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept override;
    void onDraw(gerium_uint32_t firstVertex,
                gerium_uint32_t vertexCount,
                gerium_uint32_t firstInstance,
                gerium_uint32_t instanceCount) noexcept override;

    Device* _device{};
    VkCommandBuffer _commandBuffer{};
    FrameGraph* _currentFrameGraph{};
    RenderPassHandle _currentRenderPass{ Undefined };
    FramebufferHandle _currentFramebuffer{ Undefined };
    PipelineHandle _currentPipeline{ Undefined };
    VkClearValue _clearColors[kMaxImageOutputs]{};
    VkClearValue _clearDepthStencil{};
    gerium_uint16_t _framebufferHeight{};
    bool _recording{};
};

class CommandBufferPool final {
public:
    CommandBufferPool() = default;
    ~CommandBufferPool();

    CommandBufferPool(const CommandBufferPool&)  = delete;
    CommandBufferPool(CommandBufferPool&& other) = delete;

    CommandBufferPool& operator=(const CommandBufferPool&)  = delete;
    CommandBufferPool& operator=(CommandBufferPool&& other) = delete;

    void create(Device& device, gerium_uint32_t numThreads, gerium_uint32_t numBuffersPerFrame, gerium_uint32_t family);
    void destroy() noexcept;

    CommandBuffer* getPrimary(gerium_uint32_t frame, bool profile);
    CommandBuffer* getSecondary(gerium_uint32_t frame,
                                gerium_uint32_t thread,
                                RenderPassHandle renderPass,
                                FramebufferHandle framebuffer,
                                bool profile);

private:
    gerium_uint32_t getPoolIndex(gerium_uint32_t frame, gerium_uint32_t thread) const noexcept;

    Device* _device{};
    gerium_uint32_t _threadCount{};
    gerium_uint32_t _buffersPerFrame{};
    std::vector<VkCommandPool> _vkCommandPools;
    std::vector<CommandBuffer> _commandBuffers;
    std::vector<gerium_uint8_t> _indices;
};

} // namespace gerium::vulkan

#endif
