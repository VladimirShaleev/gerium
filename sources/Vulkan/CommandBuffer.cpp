#include "CommandBuffer.hpp"
#include "Device.hpp"

namespace gerium::vulkan {

CommandBuffer::CommandBuffer(Device& device, VkCommandBuffer commandBuffer) :
    _device(&device),
    _commandBuffer(commandBuffer) {
}

CommandBuffer::~CommandBuffer() {
}

void CommandBuffer::addImageBarrier(TextureHandle handle,
                                    ResourceState oldState,
                                    ResourceState newState,
                                    gerium_uint32_t mipLevel,
                                    gerium_uint32_t mipCount,
                                    bool isDepth) {
    auto& texture = _device->_textures.access(handle);

    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.srcAccessMask                   = toVkAccessFlags(oldState);
    barrier.dstAccessMask                   = toVkAccessFlags(newState);
    barrier.oldLayout                       = toVkImageLayout(oldState);
    barrier.newLayout                       = toVkImageLayout(newState);
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = texture.vkImage;
    barrier.subresourceRange.aspectMask     = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = mipLevel;
    barrier.subresourceRange.levelCount     = mipCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    const auto srcStageMask = utilDeterminePipelineStageFlags(barrier.srcAccessMask, QueueType::Graphics);
    const auto dstStageMask = utilDeterminePipelineStageFlags(barrier.dstAccessMask, QueueType::Graphics);

    _device->vkTable().vkCmdPipelineBarrier(
        _commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void CommandBuffer::submit(QueueType queue) {
    end();

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &_commandBuffer;

    VkQueue vkQueue;
    switch (queue) {
        case QueueType::Graphics:
            vkQueue = _device->_queueGraphic;
            break;
        case QueueType::Compute:
            vkQueue = _device->_queueCompute;
            break;
        case QueueType::CopyTransfer:
            vkQueue = _device->_queueTransfer;
            break;
    }

    _device->vkTable().vkQueueSubmit(vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    _device->vkTable().vkQueueWaitIdle(vkQueue);
}

void CommandBuffer::reset() {
    // if (_vkDescriptorPool) {
    //     _vkTable->vkResetDescriptorPool(_device, _vkDescriptorPool, 0);
    // }
}

void CommandBuffer::begin() {
    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    _device->vkTable().vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

void CommandBuffer::end() {
    _device->vkTable().vkEndCommandBuffer(_commandBuffer);
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
    commandBuffer.reset();
    commandBuffer.begin();

    return &commandBuffer;
}

uint32_t CommandBufferManager::getPoolIndex(uint32_t frame, uint32_t thread) const noexcept {
    return frame * _poolsPerFrame + thread;
}

} // namespace gerium::vulkan
