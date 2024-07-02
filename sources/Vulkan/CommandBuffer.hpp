#ifndef GERIUM_WINDOWS_VULKAN_COMMAND_BUFFER_HPP
#define GERIUM_WINDOWS_VULKAN_COMMAND_BUFFER_HPP

#include "../Gerium.hpp"
#include "Utils.hpp"

namespace gerium::vulkan {

class Device;
class CommandBufferManager;

class CommandBuffer final {
public:
    CommandBuffer(Device& device, VkCommandBuffer commandBUffer);
    ~CommandBuffer();

private:
    friend CommandBufferManager;

    void reset();
    void begin();

    Device* _device;
    VkCommandBuffer _commandBUffer;
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

    CommandBuffer* getCommandBuffer(uint32_t frame, uint32_t thread);

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
