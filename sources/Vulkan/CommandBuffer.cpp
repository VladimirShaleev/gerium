#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "VkRenderer.hpp"

namespace gerium::vulkan {

CommandBuffer::CommandBuffer(Device& device, VkCommandBuffer commandBuffer) :
    _device(&device),
    _commandBuffer(commandBuffer),
    _currentRenderPass(Undefined),
    _currentFramebuffer(Undefined),
    _currentPipeline(Undefined) {
}

CommandBuffer::~CommandBuffer() {
}

void CommandBuffer::addImageBarrier(TextureHandle handle,
                                    ResourceState oldState,
                                    ResourceState newState,
                                    gerium_uint32_t mipLevel,
                                    gerium_uint32_t mipCount,
                                    bool isDepth,
                                    bool isStencil) {
    auto texture = _device->_textures.access(handle);

    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.srcAccessMask                   = toVkAccessFlags(oldState);
    barrier.dstAccessMask                   = toVkAccessFlags(newState);
    barrier.oldLayout                       = toVkImageLayout(oldState);
    barrier.newLayout                       = toVkImageLayout(newState);
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = texture->vkImage;
    barrier.subresourceRange.aspectMask     = isDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = mipLevel;
    barrier.subresourceRange.levelCount     = mipCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    if (isStencil) {
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    const auto srcStageMask = utilDeterminePipelineStageFlags(barrier.srcAccessMask, QueueType::Graphics);
    const auto dstStageMask = utilDeterminePipelineStageFlags(barrier.dstAccessMask, QueueType::Graphics);

    _device->vkTable().vkCmdPipelineBarrier(
        _commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void CommandBuffer::clearColor(gerium_uint32_t index,
                               gerium_float32_t red,
                               gerium_float32_t green,
                               gerium_float32_t blue,
                               gerium_float32_t alpha) noexcept {
    _clearColors[index].color = { red, green, blue, alpha };
}

void CommandBuffer::clearDepthStencil(gerium_float32_t depth, gerium_uint32_t value) noexcept {
    _clearDepthStencil.depthStencil = { depth, value };
}

void CommandBuffer::bindPass(RenderPassHandle renderPass, FramebufferHandle framebuffer) {
    auto renderPassObj  = _device->_renderPasses.access(renderPass);
    auto framebufferObj = _device->_framebuffers.access(framebuffer);

    if (_currentRenderPass != renderPass) {
        endCurrentRenderPass();

        uint32_t clearValuesCount = 0;
        VkClearValue clearValues[kMaxImageOutputs + 1];

        for (uint32_t o = 0; o < renderPassObj->output.numColorFormats; ++o) {
            if (renderPassObj->output.colorOperations[o] == GERIUM_RENDER_PASS_OP_CLEAR) {
                clearValues[clearValuesCount++] = _clearColors[o];
            }
        }

        if (renderPassObj->output.depthStencilFormat != VK_FORMAT_UNDEFINED) {
            if (renderPassObj->output.depthOperation == GERIUM_RENDER_PASS_OP_CLEAR) {
                clearValues[clearValuesCount++] = _clearDepthStencil;
            }
        }

        VkRenderPassBeginInfo renderPassBegin{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBegin.framebuffer       = framebufferObj->vkFramebuffer;
        renderPassBegin.renderPass        = renderPassObj->vkRenderPass;
        renderPassBegin.renderArea.offset = { 0, 0 };
        renderPassBegin.renderArea.extent = { framebufferObj->width, framebufferObj->height };
        renderPassBegin.clearValueCount   = clearValuesCount;
        renderPassBegin.pClearValues      = clearValues;
        _device->vkTable().vkCmdBeginRenderPass(_commandBuffer,
                                                &renderPassBegin,
                                                false ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                                      : VK_SUBPASS_CONTENTS_INLINE);
        _currentRenderPass  = renderPass;
        _currentFramebuffer = framebuffer;
    }
}

// TODO: remove later
void CommandBuffer::bindPipeline(PipelineHandle pipeline) {
    auto pipelineObj = _device->_pipelines.access(pipeline);

    if (_currentPipeline != pipeline) {
        _device->vkTable().vkCmdBindPipeline(_commandBuffer, pipelineObj->vkBindPoint, pipelineObj->vkPipeline);
        _currentPipeline = pipeline;
    }
}

void CommandBuffer::copyBuffer(BufferHandle src, BufferHandle dst) {
    auto srcBuffer = _device->_buffers.access(src);
    auto dstBuffer = _device->_buffers.access(dst);

    auto srcOffset = 0;
    auto srcSize   = srcBuffer->size;

    if (srcBuffer->parent != Undefined) {
        srcBuffer = _device->_buffers.access(srcBuffer->parent);
        srcOffset = srcBuffer->globalOffset;
    }

    VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
    barrier.srcAccessMask       = VK_ACCESS_HOST_WRITE_BIT;
    barrier.dstAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer              = srcBuffer->vkBuffer;
    barrier.offset              = srcOffset;
    barrier.size                = srcSize;

    const auto srcStageMask = utilDeterminePipelineStageFlags(barrier.srcAccessMask, QueueType::Graphics);
    const auto dstStageMask = utilDeterminePipelineStageFlags(barrier.dstAccessMask, QueueType::Graphics);

    _device->vkTable().vkCmdPipelineBarrier(
        _commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 1, &barrier, 0, nullptr);

    VkBufferCopy bufferCopy = { (VkDeviceSize) srcOffset, 0, srcSize };
    _device->vkTable().vkCmdCopyBuffer(_commandBuffer, srcBuffer->vkBuffer, dstBuffer->vkBuffer, 1, &bufferCopy);

    VkBufferMemoryBarrier barrier2{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
    barrier2.srcAccessMask       = VK_ACCESS_TRANSFER_READ_BIT;
    barrier2.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
    barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.buffer              = srcBuffer->vkBuffer;
    barrier2.offset              = srcOffset;
    barrier2.size                = srcSize;

    const auto srcStageMask2 = utilDeterminePipelineStageFlags(barrier2.srcAccessMask, QueueType::Graphics);
    const auto dstStageMask2 = utilDeterminePipelineStageFlags(barrier2.dstAccessMask, QueueType::Graphics);

    _device->vkTable().vkCmdPipelineBarrier(
        _commandBuffer, srcStageMask2, dstStageMask2, 0, 0, nullptr, 1, &barrier2, 0, nullptr);
}

void CommandBuffer::pushMarker(gerium_utf8_t name) {
    if (_device->_profilerEnabled) {
        auto queryIndex = profiler()->pushTimestamp(name);
        _device->vkTable().vkCmdWriteTimestamp(
            _commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, _device->_queryPool, queryIndex);
    }
}

void CommandBuffer::popMarker() {
    if (_device->_profilerEnabled) {
        auto queryIndex = profiler()->popTimestamp();
        _device->vkTable().vkCmdWriteTimestamp(
            _commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, _device->_queryPool, queryIndex);
    }
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

void CommandBuffer::endCurrentRenderPass() {
    if (_currentRenderPass != Undefined) {
        _device->vkTable().vkCmdEndRenderPass(_commandBuffer);
        _currentRenderPass  = Undefined;
        _currentFramebuffer = Undefined;
    }
}

void CommandBuffer::onSetViewport(gerium_uint16_t x,
                                  gerium_uint16_t y,
                                  gerium_uint16_t width,
                                  gerium_uint16_t height,
                                  gerium_float32_t minDepth,
                                  gerium_float32_t maxDepth) noexcept {
    VkViewport viewport{};
    viewport.x        = float(x);
    viewport.width    = float(width);
    viewport.y        = float(gerium_sint32_t(_framebufferHeight) - gerium_sint32_t(y));
    viewport.height   = -float(height);
    viewport.minDepth = minDepth;
    viewport.maxDepth = maxDepth;

    _device->vkTable().vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);
}

void CommandBuffer::onSetScissor(gerium_uint16_t x,
                                 gerium_uint16_t y,
                                 gerium_uint16_t width,
                                 gerium_uint16_t height) noexcept {
    VkRect2D scissor{ x, gerium_sint32_t(_framebufferHeight - height) - gerium_sint32_t(y), width, height };
    _device->vkTable().vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);
}

void CommandBuffer::onBindMaterial(MaterialHandle handle) noexcept {
    auto pipeline = alias_cast<VkRenderer*>(getRenderer())->getPipeline(handle);

    if (_currentPipeline != pipeline) {
        auto pipelineObj = _device->_pipelines.access(pipeline);
        _device->vkTable().vkCmdBindPipeline(_commandBuffer, pipelineObj->vkBindPoint, pipelineObj->vkPipeline);
        _currentPipeline = pipeline;
    }
}

void CommandBuffer::onBindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept {
    auto buffer = _device->_buffers.access(handle);

    VkBuffer vkBuffer     = buffer->vkBuffer;
    VkDeviceSize vkOffset = offset;
    if (buffer->parent != Undefined) {
        auto parentBuffer = _device->_buffers.access(buffer->parent);
        vkBuffer          = parentBuffer->vkBuffer;
        vkOffset          = buffer->globalOffset;
    }
    _device->vkTable().vkCmdBindVertexBuffers(_commandBuffer, binding, 1, &vkBuffer, &vkOffset);
}

void CommandBuffer::onBindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept {
    auto pipeline      = _device->_pipelines.access(_currentPipeline);
    auto descriptorSet = _device->_descriptorSets.access(handle);
    auto layoutHandle  = pipeline->descriptorSetLayoutHandles[set];
    auto layout        = _device->_descriptorSetLayouts.access(layoutHandle);

    auto vkDescriptorSet = _device->updateDescriptorSet(handle, layoutHandle, _currentFrameGraph);

    uint32_t offsets[kMaxDescriptorsPerSet];
    gerium_uint32_t bufferCount = 0;

    for (const auto& binding : layout->data.bindings) {
        if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
            offsets[bufferCount] = _device->_buffers.access(descriptorSet->bindings[binding.binding])->globalOffset;
            ++bufferCount;
        }
    }

    _device->vkTable().vkCmdBindDescriptorSets(_commandBuffer,
                                               pipeline->vkBindPoint,
                                               pipeline->vkPipelineLayout,
                                               set,
                                               1,
                                               &vkDescriptorSet,
                                               bufferCount,
                                               offsets);
}

void CommandBuffer::onDraw(gerium_uint32_t firstVertex,
                           gerium_uint32_t vertexCount,
                           gerium_uint32_t firstInstance,
                           gerium_uint32_t instanceCount) noexcept {
    _device->vkTable().vkCmdDraw(_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

VkProfiler* CommandBuffer::profiler() noexcept {
    return _device->_profiler.get();
}

void CommandBuffer::reset() {
    // if (_vkDescriptorPool) {
    //     _vkTable->vkResetDescriptorPool(_device, _vkDescriptorPool, 0);
    // }
}

void CommandBuffer::begin() {
    _currentRenderPass  = Undefined;
    _currentFramebuffer = Undefined;
    _currentPipeline    = Undefined;

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    _device->vkTable().vkBeginCommandBuffer(_commandBuffer, &beginInfo);
}

void CommandBuffer::end() {
    endCurrentRenderPass();
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

CommandBuffer* CommandBufferManager::getCommandBuffer(uint32_t frame, uint32_t thread, bool profile) {
    const auto poolIndex = getPoolIndex(frame, thread);
    assert(_usedBuffers[poolIndex] < CommandBuffersPerThread);

    const auto offset   = _usedBuffers[poolIndex]++;
    auto& commandBuffer = _commandBuffers[poolIndex * CommandBuffersPerThread + offset];
    commandBuffer.reset();
    commandBuffer.begin();

    if (_device->_profilerEnabled && profile && !_device->_profiler->hasTimestamps()) {
        const auto queriesPerFrame = _device->_profiler->queriesPerFrame();
        _device->vkTable().vkCmdResetQueryPool(
            commandBuffer._commandBuffer, _device->_queryPool, frame * queriesPerFrame * 2, queriesPerFrame);
    }

    return &commandBuffer;
}

uint32_t CommandBufferManager::getPoolIndex(uint32_t frame, uint32_t thread) const noexcept {
    return frame * _poolsPerFrame + thread;
}

} // namespace gerium::vulkan
