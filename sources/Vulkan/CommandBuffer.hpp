/*
 * The current command pool implementation is inspired from this project:
 *   https://github.com/PacktPublishing/Mastering-Graphics-Programming-with-Vulkan
 */

#ifndef GERIUM_WINDOWS_VULKAN_COMMAND_BUFFER_HPP
#define GERIUM_WINDOWS_VULKAN_COMMAND_BUFFER_HPP

#include "../CommandBuffer.hpp"
#include "../Gerium.hpp"
#include "Resources.hpp"
#include "Utils.hpp"
#include "VkProfiler.hpp"

namespace gerium::vulkan {

class Device;
class VkRenderer;
class CommandBufferManager;

class CommandBuffer final : public gerium::CommandBuffer {
public:
    CommandBuffer(Device& device, VkCommandBuffer commandBuffer);
    ~CommandBuffer();

    void addImageBarrier(TextureHandle handle,
                         ResourceState oldState,
                         ResourceState newState,
                         gerium_uint32_t mipLevel,
                         gerium_uint32_t mipCount,
                         bool isDepth,
                         bool isStencil);
    void setScissor(const Rect2DInt* rect);
    void setViewport(const Viewport* viewport);
    void bindPass(RenderPassHandle renderPass, FramebufferHandle framebuffer);
    void bindPipeline(PipelineHandle pipeline);
    void copyBuffer(BufferHandle src, BufferHandle dst);
    void pushMarker(gerium_utf8_t name);
    void popMarker();
    void submit(QueueType queue);

    void endCurrentRenderPass();

    VkCommandBuffer vkCommandBuffer() noexcept {
        return _commandBuffer;
    }

private:
    friend Device;
    friend CommandBufferManager;

    void onClearColor(gerium_uint32_t index,
                      gerium_float32_t red,
                      gerium_float32_t green,
                      gerium_float32_t blue,
                      gerium_float32_t alpha) noexcept override;
    void onClearDepthStencil(gerium_float32_t depth, gerium_uint32_t value) noexcept override;

    void onBindMaterial(MaterialHandle handle) noexcept override;
    void onBindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept override;
    void onBindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept override;
    void onDraw(gerium_uint32_t firstVertex,
                gerium_uint32_t vertexCount,
                gerium_uint32_t firstInstance,
                gerium_uint32_t instanceCount) noexcept override;

    VkProfiler* profiler() noexcept;

    void reset();
    void begin();
    void end();

    Device* _device;
    VkCommandBuffer _commandBuffer;
    RenderPassHandle _currentRenderPass;
    FramebufferHandle _currentFramebuffer;
    PipelineHandle _currentPipeline;
    VkClearValue _clearColors[kMaxImageOutputs]{};
    VkClearValue _clearDepthStencil{};
};

class CommandBufferManager final {
public:
    CommandBufferManager() = default;
    ~CommandBufferManager();

    CommandBufferManager(const CommandBufferManager&)            = delete;
    CommandBufferManager& operator=(const CommandBufferManager&) = delete;

    CommandBufferManager(CommandBufferManager&& other) noexcept :
        _device(other._device),
        _poolsPerFrame(other._poolsPerFrame) {
        other._device        = nullptr;
        other._poolsPerFrame = 0;
        _vkCommandPools.swap(other._vkCommandPools);
        _commandBuffers.swap(other._commandBuffers);
        _usedBuffers.swap(other._usedBuffers);
    }

    CommandBufferManager& operator=(CommandBufferManager&& other) noexcept {
        if (this != &other) {
            destroy();
            std::swap(_device, other._device);
            std::swap(_poolsPerFrame, other._poolsPerFrame);
            _vkCommandPools.swap(other._vkCommandPools);
            _commandBuffers.swap(other._commandBuffers);
            _usedBuffers.swap(other._usedBuffers);
        }
        return *this;
    }

    void create(Device& device, uint32_t numThreads, uint32_t family);
    void destroy() noexcept;

    void newFrame() noexcept;

    CommandBuffer* getCommandBuffer(uint32_t frame, uint32_t thread, bool profile);

private:
    uint32_t getPoolIndex(uint32_t frame, uint32_t thread) const noexcept;

    static constexpr uint32_t CommandBuffersPerThread = 4;

    Device* _device{};
    uint32_t _poolsPerFrame{};
    std::vector<VkCommandPool> _vkCommandPools;
    std::vector<CommandBuffer> _commandBuffers;
    std::vector<uint8_t> _usedBuffers;
};

} // namespace gerium::vulkan

#endif
