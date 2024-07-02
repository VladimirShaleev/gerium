#include "CommandBuffer.hpp"
#include "Device.hpp"

namespace gerium::vulkan {

CommandBuffer::CommandBuffer(Device& device, VkCommandBuffer commandBUffer) :
    _device(&device),
    _commandBUffer(commandBUffer) {
}

CommandBuffer::~CommandBuffer() {
}

void CommandBuffer::reset() {
    // if (_vkDescriptorPool) {
    //     _vkTable->vkResetDescriptorPool(_device, _vkDescriptorPool, 0);
    // }
}

void CommandBuffer::begin() {
    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    _device->vkTable().vkBeginCommandBuffer(_commandBUffer, &beginInfo);
}

CommandBufferManager::~CommandBufferManager() {
    destroy();
}

void CommandBufferManager::create(Device& device, uint32_t numThreads, uint32_t family) {
    _device        = &device;
    _poolsPerFrame = numThreads;

    const auto totalPools = _poolsPerFrame * _device->MaxFrames;

    _vkCommandPools.resize(totalPools);
    _usedBuffers.resize(totalPools);

    for (uint32_t i = 0; i < totalPools; ++i) {
        VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = family;
        check(_device->vkTable().vkCreateCommandPool(
            _device->vkDevice(), &poolInfo, getAllocCalls(), &_vkCommandPools[i]));
    }

    _commandBuffers.reserve(totalPools * CommandBuffersPerThread);
    VkCommandBuffer buffers[CommandBuffersPerThread] = {};

    for (uint32_t frame = 0; frame < device.MaxFrames; ++frame) {
        for (uint32_t thread = 0; thread < numThreads; ++thread) {
            const auto poolIndex = getPoolIndex(frame, thread);

            VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandPool        = _vkCommandPools[poolIndex];
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = CommandBuffersPerThread;
            check(_device->vkTable().vkAllocateCommandBuffers(_device->vkDevice(), &allocInfo, buffers));

            for (uint32_t i = 0; i < CommandBuffersPerThread; ++i) {
                _commandBuffers.emplace_back(*_device, buffers[i]);
            }
        }
    }
}

void CommandBufferManager::destroy() noexcept {
    for (auto& pool : _vkCommandPools) {
        _device->vkTable().vkDestroyCommandPool(_device->vkDevice(), pool, getAllocCalls());
    }

    _commandBuffers.clear();
    _vkCommandPools.clear();
    _usedBuffers.clear();
}

void CommandBufferManager::newFrame() noexcept {
    memset(_usedBuffers.data(), 0, _usedBuffers.size());
}

CommandBuffer* CommandBufferManager::getCommandBuffer(uint32_t frame, uint32_t thread) {
    const auto poolIndex = getPoolIndex(frame, thread);
    assert(_usedBuffers[poolIndex] < CommandBuffersPerThread);

    const auto offset   = _usedBuffers[poolIndex]++;
    auto& commandBuffer = _commandBuffers[poolIndex * CommandBuffersPerThread + offset];
    // commandBuffer.reset();
    // commandBuffer.begin();

    return &commandBuffer;
}

uint32_t CommandBufferManager::getPoolIndex(uint32_t frame, uint32_t thread) const noexcept {
    return frame * _poolsPerFrame + thread;
}

} // namespace gerium::vulkan
