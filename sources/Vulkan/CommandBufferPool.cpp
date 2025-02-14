#include "CommandBufferPool.hpp"
#include "Device.hpp"
#include "VkRenderer.hpp"

namespace gerium::vulkan {

CommandBuffer::CommandBuffer(Device& device, VkCommandBuffer commandBuffer) :
    _device(&device),
    _commandBuffer(commandBuffer) {
    for (auto& set : _currentDescriptorSets) {
        set = Undefined;
    }
    memset(_currentDescriptorSetsChanged, 0, sizeof(_currentDescriptorSetsChanged));
}

void CommandBuffer::addImageBarrier(TextureHandle handle,
                                    ResourceState newState,
                                    gerium_uint32_t mipLevel,
                                    gerium_uint32_t mipCount,
                                    QueueType srcQueueType,
                                    QueueType dstQueueType) {
    auto texture = _device->_textures.access(handle);
    auto states  = getTextureStates(handle);

    if (mipLevel == 0 && mipCount == 1 && states[0] == newState) {
        return;
    }

    auto srcFamily = srcQueueType == dstQueueType ? VK_QUEUE_FAMILY_IGNORED : getFamilyIndex(srcQueueType);
    auto dstFamily = srcQueueType == dstQueueType ? VK_QUEUE_FAMILY_IGNORED : getFamilyIndex(dstQueueType);

    VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.srcAccessMask                   = toVkAccessFlags(states[mipLevel]);
    barrier.dstAccessMask                   = toVkAccessFlags(newState);
    barrier.oldLayout                       = toVkImageLayout(states[mipLevel]);
    barrier.newLayout                       = toVkImageLayout(newState);
    barrier.srcQueueFamilyIndex             = srcFamily;
    barrier.dstQueueFamilyIndex             = dstFamily;
    barrier.image                           = texture->vkImage;
    barrier.subresourceRange.aspectMask     = toVkImageAspect(texture->vkFormat);
    barrier.subresourceRange.baseMipLevel   = mipLevel;
    barrier.subresourceRange.levelCount     = mipCount == 0 ? texture->mipLevels : mipCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = texture->layers;

    if (hasStencil(texture->vkFormat)) {
        barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    const auto srcStageMask = utilDeterminePipelineStageFlags(barrier.srcAccessMask, srcQueueType);
    const auto dstStageMask = utilDeterminePipelineStageFlags(barrier.dstAccessMask, dstQueueType);

    _device->vkTable().vkCmdPipelineBarrier(
        _commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    for (gerium_uint32_t mip = mipLevel; mip < mipLevel + mipCount; ++mip) {
        states[mip] = newState;
    }
}

void CommandBuffer::addBufferBarrier(BufferHandle handle,
                                     ResourceState dstState,
                                     gerium_uint32_t offset,
                                     gerium_uint32_t size) {
    auto buffer = _device->_buffers.access(handle);

    auto [vkBuffer, vkOffset] = getVkBuffer(handle, offset);

    VkBufferMemoryBarrier barrier{ VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER };
    barrier.srcAccessMask       = toVkAccessFlags(buffer->state);
    barrier.dstAccessMask       = toVkAccessFlags(dstState);
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer              = vkBuffer;
    barrier.offset              = vkOffset;
    barrier.size                = size == 0 ? buffer->size : std::min(size, buffer->size);

    const auto srcStageMask = utilDeterminePipelineStageFlags(barrier.srcAccessMask, QueueType::Graphics);
    const auto dstStageMask = utilDeterminePipelineStageFlags(barrier.dstAccessMask, QueueType::Graphics);

    _device->vkTable().vkCmdPipelineBarrier(
        _commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 1, &barrier, 0, nullptr);
    buffer->state = dstState;
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

void CommandBuffer::bindPass(RenderPassHandle renderPass,
                             FramebufferHandle framebuffer,
                             bool useSecondaryCommandBuffers) {
    if (_currentRenderPass != renderPass) {
        endCurrentRenderPass();

        auto renderPassObj  = _device->_renderPasses.access(renderPass);
        auto framebufferObj = _device->_framebuffers.access(framebuffer);

        gerium_uint32_t clearValuesCount = 0;
        VkClearValue clearValues[kMaxImageOutputs + 1];

        for (gerium_uint32_t o = 0; o < renderPassObj->output.numColorFormats; ++o) {
            if (renderPassObj->output.colorOperations[o] == RenderPassOp::Clear) {
                clearValues[clearValuesCount++] = _clearColors[o];
            }
        }

        if (renderPassObj->output.depthStencilFormat != VK_FORMAT_UNDEFINED) {
            if (renderPassObj->output.depthOperation == RenderPassOp::Clear) {
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
        _device->vkTable().vkCmdBeginRenderPass(
            _commandBuffer,
            &renderPassBegin,
            useSecondaryCommandBuffers ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS : VK_SUBPASS_CONTENTS_INLINE);

        _currentRenderPass  = renderPass;
        _currentFramebuffer = framebuffer;
    }
}

void CommandBuffer::copyBuffer(BufferHandle src, BufferHandle dst) {
    auto srcBuffer = _device->_buffers.access(src);
    auto dstBuffer = _device->_buffers.access(dst);

    auto srcOffset = srcBuffer->globalOffset;
    auto srcSize   = srcBuffer->size;

    if (srcBuffer->parent != Undefined) {
        srcBuffer = _device->_buffers.access(srcBuffer->parent);
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

void CommandBuffer::copyBuffer(BufferHandle src, TextureHandle dst, gerium_uint8_t mip, gerium_uint32_t offset) {
    auto srcBuffer  = _device->_buffers.access(src);
    auto dstTexture = _device->_textures.access(dst);
    auto srcOffset  = srcBuffer->globalOffset + offset;

    auto isTransfer = srcBuffer->vkUsageFlags == VkBufferUsageFlagBits{};
    // auto srcFamily  = isTransfer ? _device->_queueFamilies.transfer.value().index : VK_QUEUE_FAMILY_IGNORED;
    // auto dstFamily  = isTransfer ? _device->_queueFamilies.graphic.value().index : VK_QUEUE_FAMILY_IGNORED;
    auto queue = isTransfer ? QueueType::CopyTransfer : QueueType::Graphics;

    if (srcBuffer->parent != Undefined) {
        srcBuffer = _device->_buffers.access(srcBuffer->parent);
    }

    const auto width  = std::max(static_cast<uint32_t>(dstTexture->width * std::pow(0.5, mip)), 1U);
    const auto height = std::max(static_cast<uint32_t>(dstTexture->height * std::pow(0.5, mip)), 1U);
    const auto depth  = std::max(static_cast<uint32_t>(dstTexture->depth * std::pow(0.5, mip)), 1U);

    VkBufferImageCopy region{};
    region.bufferOffset                    = (VkDeviceSize) srcOffset;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = toVkImageAspect(dstTexture->vkFormat);
    region.imageSubresource.mipLevel       = mip;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = { 0, 0, 0 };
    region.imageExtent                     = { width, height, depth };

    addImageBarrier(dst, ResourceState::CopyDest, mip, 1, queue, queue);
    _device->vkTable().vkCmdCopyBufferToImage(
        _commandBuffer, srcBuffer->vkBuffer, dstTexture->vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
}

void CommandBuffer::generateMipmaps(TextureHandle handle) {
    auto texture = _device->_textures.access(handle);

    int32_t w = texture->width;
    int32_t h = texture->height;

    addImageBarrier(handle, ResourceState::CopySource, 0, 1);
    for (int mipIndex = 1; mipIndex < texture->mipLevels; ++mipIndex) {
        addImageBarrier(handle, ResourceState::CopyDest, mipIndex, 1);

        VkImageBlit blitRegion{};
        blitRegion.srcSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.mipLevel       = mipIndex - 1;
        blitRegion.srcSubresource.baseArrayLayer = 0;
        blitRegion.srcSubresource.layerCount     = 1;
        blitRegion.srcOffsets[0]                 = { 0, 0, 0 };
        blitRegion.srcOffsets[1]                 = { w, h, 1 };
        blitRegion.dstSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.mipLevel       = mipIndex;
        blitRegion.dstSubresource.baseArrayLayer = 0;
        blitRegion.dstSubresource.layerCount     = 1;
        blitRegion.dstOffsets[0]                 = { 0, 0, 0 };
        blitRegion.dstOffsets[1]                 = { w /= 2, h /= 2, 1 };

        _device->_vkTable.vkCmdBlitImage(_commandBuffer,
                                         texture->vkImage,
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         texture->vkImage,
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                         1,
                                         &blitRegion,
                                         VK_FILTER_LINEAR);

        addImageBarrier(handle, ResourceState::CopySource, mipIndex, 1);
    }

    addImageBarrier(handle, ResourceState::ShaderResource, 0, texture->mipLevels);
}

void CommandBuffer::pushMarker(gerium_utf8_t name) {
    if (_device->isProfilerEnable()) {
        auto queryIndex = _device->profiler()->pushTimestamp(name);
        _device->vkTable().vkCmdWriteTimestamp(
            _commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, _device->_queryPool, queryIndex);
    }
}

void CommandBuffer::popMarker() {
    if (_device->isProfilerEnable()) {
        auto queryIndex = _device->profiler()->popTimestamp();
        _device->vkTable().vkCmdWriteTimestamp(
            _commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, _device->_queryPool, queryIndex);
    }
}

void CommandBuffer::pushLabel(gerium_utf8_t name) {
    if (_device->_enableDebugNames) {
        const auto color = nameColorVec4(name);
        VkDebugUtilsLabelEXT label{};
        label.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
        label.pLabelName = name;
        label.color[0]   = color.r;
        label.color[1]   = color.g;
        label.color[2]   = color.b;
        label.color[3]   = color.a;
        _device->vkTable().vkCmdBeginDebugUtilsLabelEXT(_commandBuffer, &label);
    }
}

void CommandBuffer::popLabel() {
    if (_device->_enableDebugNames) {
        _device->vkTable().vkCmdEndDebugUtilsLabelEXT(_commandBuffer);
    }
}

void CommandBuffer::submit(QueueType queue, bool wait) {
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

    if (wait) {
        _device->vkTable().vkQueueWaitIdle(vkQueue);
    }
}

void CommandBuffer::execute(gerium_uint32_t numCommandBuffers, CommandBuffer* commandBuffers[]) {
    VkCommandBuffer buffers[100];
    for (gerium_uint32_t i = 0; i < numCommandBuffers; ++i) {
        commandBuffers[i]->end();
        buffers[i] = commandBuffers[i]->vkCommandBuffer();
    }
    _device->vkTable().vkCmdExecuteCommands(_commandBuffer, numCommandBuffers, buffers);
}

void CommandBuffer::begin(RenderPassHandle renderPass, FramebufferHandle framebuffer) {
    const auto isSecondary = renderPass != Undefined && framebuffer != Undefined;

    _currentRenderPass  = Undefined;
    _currentFramebuffer = Undefined;
    _currentPipeline    = Undefined;

    VkCommandBufferInheritanceInfo inheritanceInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
    if (isSecondary) {
        auto renderPassObj  = _device->_renderPasses.access(renderPass);
        auto framebufferObj = _device->_framebuffers.access(framebuffer);

        inheritanceInfo.renderPass  = renderPassObj->vkRenderPass;
        inheritanceInfo.subpass     = 0;
        inheritanceInfo.framebuffer = framebufferObj->vkFramebuffer;
    }

    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT |
                      (isSecondary ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0);
    beginInfo.pInheritanceInfo = isSecondary ? &inheritanceInfo : nullptr;
    _device->vkTable().vkBeginCommandBuffer(_commandBuffer, &beginInfo);

    _recording = true;
}

void CommandBuffer::end() {
    _device->vkTable().vkEndCommandBuffer(_commandBuffer);

    _recording = false;
}

void CommandBuffer::endCurrentRenderPass() {
    if (_currentRenderPass != Undefined) {
        _device->vkTable().vkCmdEndRenderPass(_commandBuffer);
        _currentRenderPass  = Undefined;
        _currentFramebuffer = Undefined;
    }
}

VkCommandBuffer CommandBuffer::vkCommandBuffer() const noexcept {
    return _commandBuffer;
}

void CommandBuffer::setFramebufferHeight(gerium_uint16_t framebufferHeight) noexcept {
    _framebufferHeight = framebufferHeight;
}

void CommandBuffer::setFrameGraph(FrameGraph* frameGraph) noexcept {
    _currentFrameGraph = frameGraph;
}

bool CommandBuffer::isRecording() const noexcept {
    return _recording;
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

void CommandBuffer::onBindTechnique(TechniqueHandle handle) noexcept {
    auto pipeline = alias_cast<VkRenderer*>(getRenderer())->getPipeline(handle);

    if (_currentPipeline != pipeline) {
        auto pipelineObj = _device->_pipelines.access(pipeline);
        _device->vkTable().vkCmdBindPipeline(_commandBuffer, pipelineObj->vkBindPoint, pipelineObj->vkPipeline);
        _currentPipeline = pipeline;

        for (uint32_t i = 0; i < pipelineObj->numActiveLayouts; ++i) {
            if (pipelineObj->descriptorSetLayoutHandles[i] == Undefined) {
                _currentDescriptorSets[i]        = Undefined;
                _currentDescriptorSetsChanged[i] = false;
            } else {
                _currentDescriptorSetsChanged[i] = true;
            }
        }
        for (uint32_t i = pipelineObj->numActiveLayouts; i < std::size(_currentDescriptorSets); ++i) {
            _currentDescriptorSets[i]        = Undefined;
            _currentDescriptorSetsChanged[i] = false;
        }
    }
}

void CommandBuffer::onBindVertexBuffer(BufferHandle handle, gerium_uint32_t binding, gerium_uint32_t offset) noexcept {
    auto [vkBuffer, vkOffset] = getVkBuffer(handle, offset);
    _device->vkTable().vkCmdBindVertexBuffers(_commandBuffer, binding, 1, &vkBuffer, &vkOffset);
}

void CommandBuffer::onBindIndexBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_index_type_t type) noexcept {
    auto [vkBuffer, vkOffset] = getVkBuffer(handle, offset);
    _device->vkTable().vkCmdBindIndexBuffer(_commandBuffer, vkBuffer, vkOffset, toVkIndexType(type));
}

void CommandBuffer::onBindDescriptorSet(DescriptorSetHandle handle, gerium_uint32_t set) noexcept {
    assert(set < std::size(_currentDescriptorSets));
    if (_currentDescriptorSets[set] != handle) {
        _currentDescriptorSets[set]        = handle;
        _currentDescriptorSetsChanged[set] = true;
    }
}

void CommandBuffer::onDispatch(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept {
    bindDescriptorSets();
    _device->vkTable().vkCmdDispatch(_commandBuffer, groupX, groupY, groupZ);
}

void CommandBuffer::onDraw(gerium_uint32_t firstVertex,
                           gerium_uint32_t vertexCount,
                           gerium_uint32_t firstInstance,
                           gerium_uint32_t instanceCount) noexcept {
    bindDescriptorSets();
    _device->vkTable().vkCmdDraw(_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::onDrawIndirect(BufferHandle handle,
                                   gerium_uint32_t offset,
                                   BufferHandle drawCountHandle,
                                   gerium_uint32_t drawCountOffset,
                                   gerium_uint32_t drawCount,
                                   gerium_uint32_t stride) noexcept {
    auto [vkBuffer, vkOffset] = getVkBuffer(handle, offset);

    bindDescriptorSets();
    if (drawCountHandle == Undefined) {
        if (_device->_multiDrawIndirectSupported) {
            _device->vkTable().vkCmdDrawIndirect(_commandBuffer, vkBuffer, vkOffset, drawCount, stride);
        } else {
            for (gerium_uint32_t i = 0; i < drawCount; ++i) {
                _device->vkTable().vkCmdDrawIndirect(_commandBuffer, vkBuffer, vkOffset + i * stride, 1, stride);
            }
        }
    } else {
        auto [vkBufferCount, vkOffsetCount] = getVkBuffer(drawCountHandle, drawCountOffset);
        _device->vkTable().vkCmdDrawIndirectCount(
            _commandBuffer, vkBuffer, vkOffset, vkBufferCount, vkOffsetCount, drawCount, stride);
    }
}

void CommandBuffer::onDrawIndexed(gerium_uint32_t firstIndex,
                                  gerium_uint32_t indexCount,
                                  gerium_uint32_t vertexOffset,
                                  gerium_uint32_t firstInstance,
                                  gerium_uint32_t instanceCount) noexcept {
    bindDescriptorSets();
    _device->vkTable().vkCmdDrawIndexed(
        _commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::onDrawIndexedIndirect(BufferHandle handle,
                                          gerium_uint32_t offset,
                                          BufferHandle drawCountHandle,
                                          gerium_uint32_t drawCountOffset,
                                          gerium_uint32_t drawCount,
                                          gerium_uint32_t stride) noexcept {
    auto [vkBuffer, vkOffset] = getVkBuffer(handle, offset);

    bindDescriptorSets();
    if (drawCountHandle == Undefined) {
        if (_device->_multiDrawIndirectSupported) {
            _device->vkTable().vkCmdDrawIndexedIndirect(_commandBuffer, vkBuffer, vkOffset, drawCount, stride);
        } else {
            for (gerium_uint32_t i = 0; i < drawCount; ++i) {
                _device->vkTable().vkCmdDrawIndexedIndirect(_commandBuffer, vkBuffer, vkOffset + i * stride, 1, stride);
            }
        }
    } else {
        auto [vkBufferCount, vkOffsetCount] = getVkBuffer(drawCountHandle, drawCountOffset);
        _device->vkTable().vkCmdDrawIndexedIndirectCount(
            _commandBuffer, vkBuffer, vkOffset, vkBufferCount, vkOffsetCount, drawCount, stride);
    }
}

void CommandBuffer::onDrawMeshTasks(gerium_uint32_t groupX, gerium_uint32_t groupY, gerium_uint32_t groupZ) noexcept {
    bindDescriptorSets();
    _device->vkTable().vkCmdDrawMeshTasksEXT(_commandBuffer, groupX, groupY, groupZ);
}

void CommandBuffer::onDrawMeshTasksIndirect(BufferHandle handle,
                                            gerium_uint32_t offset,
                                            BufferHandle drawCountHandle,
                                            gerium_uint32_t drawCountOffset,
                                            gerium_uint32_t drawCount,
                                            gerium_uint32_t stride) noexcept {
    auto [vkBuffer, vkOffset] = getVkBuffer(handle, offset);

    bindDescriptorSets();

    if (drawCountHandle == Undefined) {
        if (_device->_multiDrawIndirectSupported) {
            _device->vkTable().vkCmdDrawMeshTasksIndirectEXT(_commandBuffer, vkBuffer, vkOffset, drawCount, stride);
        } else {
            for (gerium_uint32_t i = 0; i < drawCount; ++i) {
                _device->vkTable().vkCmdDrawMeshTasksIndirectEXT(
                    _commandBuffer, vkBuffer, vkOffset + i * stride, 1, stride);
            }
        }
    } else {
        auto [vkBufferCount, vkOffsetCount] = getVkBuffer(drawCountHandle, drawCountOffset);
        _device->vkTable().vkCmdDrawMeshTasksIndirectCountEXT(
            _commandBuffer, vkBuffer, vkOffset, vkBufferCount, vkOffsetCount, drawCount, stride);
    }
}

void CommandBuffer::onFillBuffer(BufferHandle handle,
                                 gerium_uint32_t offset,
                                 gerium_uint32_t size,
                                 gerium_uint32_t data) noexcept {
    auto [vkBuffer, vkOffset] = getVkBuffer(handle, offset);
    addBufferBarrier(handle, ResourceState::CopyDest, offset, size);
    _device->vkTable().vkCmdFillBuffer(_commandBuffer, vkBuffer, vkOffset, VkDeviceSize{ size }, data);
    addBufferBarrier(handle, ResourceState::ShaderResource, offset, size);
}

void CommandBuffer::onCopyBuffer(BufferHandle srcHandle,
                                 gerium_uint32_t srcOffset,
                                 BufferHandle dstHandle,
                                 gerium_uint32_t dstOffset,
                                 gerium_uint32_t size) noexcept {
    auto [vkSrcBuffer, vkSrcOffset] = getVkBuffer(srcHandle, srcOffset);
    auto [vkDstBuffer, vkDstOffset] = getVkBuffer(dstHandle, dstOffset);
    addBufferBarrier(srcHandle, ResourceState::CopySource, srcOffset, size);
    addBufferBarrier(dstHandle, ResourceState::CopyDest, dstOffset, size);

    VkBufferCopy bufferCopy = { (VkDeviceSize) vkSrcOffset, (VkDeviceSize) vkDstOffset, (VkDeviceSize) size };
    _device->vkTable().vkCmdCopyBuffer(_commandBuffer, vkSrcBuffer, vkDstBuffer, 1, &bufferCopy);

    addBufferBarrier(dstHandle, ResourceState::ShaderResource, dstOffset, size);
}

void CommandBuffer::onBarrierBufferWrite(BufferHandle handle) noexcept {
    addBufferBarrier(handle, ResourceState::UnorderedAccess);
}

void CommandBuffer::onBarrierBufferRead(BufferHandle handle) noexcept {
    addBufferBarrier(handle,
                     ResourceState::UnorderedAccess | ResourceState::IndirectArgument | ResourceState::ShaderResource);
}

void CommandBuffer::onBarrierTextureWrite(TextureHandle handle) noexcept {
    auto texture = _device->_textures.access(handle);
    addImageBarrier(handle, ResourceState::UnorderedAccess, texture->mipBase, texture->mipLevels);
}

void CommandBuffer::onBarrierTextureRead(TextureHandle handle) noexcept {
    auto texture = _device->_textures.access(handle);
    addImageBarrier(handle, ResourceState::ShaderResource, texture->mipBase, texture->mipLevels);
}

FfxCommandList CommandBuffer::onGetFfxCommandList() noexcept {
    return reinterpret_cast<FfxCommandList>(_commandBuffer);
}

void CommandBuffer::bindDescriptorSets() {
    uint32_t firstSet          = 0;
    uint32_t numDescriptorSets = 0;
    uint32_t numOffsets        = 0;
    auto pipeline              = _device->_pipelines.access(_currentPipeline);
    VkDescriptorSet descriptorSets[kMaxDescriptorSetLayouts]{};
    uint32_t offsets[kMaxDescriptorSetLayouts * kMaxDescriptorsPerSet];

    for (uint32_t set = 0; set < std::size(_currentDescriptorSets); ++set) {
        auto handle        = _currentDescriptorSets[set];
        auto changed       = _currentDescriptorSetsChanged[set];
        auto descriptorSet = handle != Undefined ? _device->_descriptorSets.access(handle) : nullptr;

        if (changed || (descriptorSet && descriptorSet->changed)) {
            if (descriptorSet) {
                auto layoutHandle    = pipeline->descriptorSetLayoutHandles[set];
                auto layout          = _device->_descriptorSetLayouts.access(layoutHandle);
                auto vkDescriptorSet = _device->updateDescriptorSet(handle, layoutHandle, _currentFrameGraph);
                for (const auto& binding : layout->data.bindings) {
                    if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                        binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC) {
                        const auto key = Device::calcBindingKey(binding.binding, 0);
                        offsets[numOffsets++] =
                            _device->_buffers.access(descriptorSet->bindings[key].handle)->globalOffset;
                    }
                }
                descriptorSets[numDescriptorSets++] = vkDescriptorSet;
            }
        } else {
            if (numDescriptorSets) {
                _device->vkTable().vkCmdBindDescriptorSets(_commandBuffer,
                                                           pipeline->vkBindPoint,
                                                           pipeline->vkPipelineLayout,
                                                           firstSet,
                                                           numDescriptorSets,
                                                           descriptorSets,
                                                           numOffsets,
                                                           offsets);
            }
            firstSet          = set + 1;
            numDescriptorSets = 0;
            numOffsets        = 0;
        }
        _currentDescriptorSetsChanged[set] = false;
    }
    if (numDescriptorSets) {
        _device->vkTable().vkCmdBindDescriptorSets(_commandBuffer,
                                                   pipeline->vkBindPoint,
                                                   pipeline->vkPipelineLayout,
                                                   firstSet,
                                                   numDescriptorSets,
                                                   descriptorSets,
                                                   numOffsets,
                                                   offsets);
    }
}

uint32_t CommandBuffer::getFamilyIndex(QueueType queue) const noexcept {
    switch (queue) {
        case QueueType::Graphics:
            return _device->_queueFamilies.graphic.value().index;
        case QueueType::Compute:
            return _device->_queueFamilies.compute.value().index;
        case QueueType::CopyTransfer:
            return _device->_queueFamilies.transfer.value().index;
    }
    assert(!"unreachable code");
    return 0;
}

std::pair<VkBuffer, VkDeviceSize> CommandBuffer::getVkBuffer(BufferHandle handle,
                                                             gerium_uint32_t offset) const noexcept {
    auto buffer = _device->_buffers.access(handle);

    VkBuffer vkBuffer     = buffer->vkBuffer;
    VkDeviceSize vkOffset = offset;

    if (buffer->parent != Undefined) {
        auto parentBuffer = _device->_buffers.access(buffer->parent);

        vkBuffer = parentBuffer->vkBuffer;
        vkOffset = buffer->globalOffset + offset;
    }

    return { vkBuffer, vkOffset };
}

ResourceState* CommandBuffer::getTextureStates(TextureHandle handle) noexcept {
    auto texture = _device->_textures.access(handle);
    auto states  = texture->states;
    if (texture->parentTexture != Undefined) {
        states = _device->_textures.access(texture->parentTexture)->states;
    }
    return states;
}

CommandBufferPool::~CommandBufferPool() {
    destroy();
}

void CommandBufferPool::create(Device& device,
                               gerium_uint32_t numThreads,
                               gerium_uint32_t numBuffersPerFrame,
                               QueueType queue) {
    _device          = &device;
    _threadCount     = numThreads + 1;
    _buffersPerFrame = numBuffersPerFrame;

    uint32_t family;
    switch (queue) {
        case QueueType::Graphics:
            family = _device->_queueFamilies.graphic.value().index;
            break;
        case QueueType::Compute:
            family = _device->_queueFamilies.compute.value().index;
            break;
        case QueueType::CopyTransfer:
            family = _device->_queueFamilies.transfer.value().index;
            break;
    }

    const auto totalPools = _threadCount * kMaxFrames;

    _vkCommandPools.resize(totalPools);

    for (gerium_uint32_t i = 0; i < totalPools; ++i) {
        VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = family;
        check(_device->vkTable().vkCreateCommandPool(
            _device->vkDevice(), &poolInfo, getAllocCalls(), &_vkCommandPools[i]));
    }

    _commandBuffers.resize(totalPools * _buffersPerFrame);
    _indices.resize(totalPools);
    VkCommandBuffer buffers[100] = {};

    for (gerium_uint32_t frame = 0; frame < kMaxFrames; ++frame) {
        for (gerium_uint32_t thread = 0; thread < _threadCount; ++thread) {
            const auto poolIndex = getPoolIndex(frame, thread);

            VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            allocInfo.commandPool = _vkCommandPools[poolIndex];
            allocInfo.level       = thread == 0 ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
            allocInfo.commandBufferCount = _buffersPerFrame;
            check(_device->vkTable().vkAllocateCommandBuffers(_device->vkDevice(), &allocInfo, buffers));

            for (gerium_uint32_t i = 0; i < _buffersPerFrame; ++i) {
                _commandBuffers[poolIndex * _buffersPerFrame + i] = CommandBuffer(*_device, buffers[i]);
            }
        }
    }
}

void CommandBufferPool::destroy() noexcept {
    for (auto& pool : _vkCommandPools) {
        _device->vkTable().vkDestroyCommandPool(_device->vkDevice(), pool, getAllocCalls());
    }

    _indices.clear();
    _commandBuffers.clear();
    _vkCommandPools.clear();
    _buffersPerFrame = 0;
    _threadCount     = 0;
    _device          = nullptr;
}

void CommandBufferPool::wait(QueueType queue) {
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
    _device->vkTable().vkQueueWaitIdle(vkQueue);
}

CommandBuffer* CommandBufferPool::getPrimary(gerium_uint32_t frame, bool profile) {
    const auto poolIndex   = getPoolIndex(frame, 0);
    const auto bufferIndex = _indices[poolIndex];

    auto& commandBuffer = _commandBuffers[poolIndex * _buffersPerFrame + bufferIndex];

    if (commandBuffer.isRecording()) {
        throw Exception(GERIUM_RESULT_ERROR_INVALID_OPERATION);
    }

    commandBuffer.begin();

    if (profile && _device->isProfilerEnable() && !_device->profiler()->hasTimestamps()) {
        const auto queriesPerFrame = _device->profiler()->queriesPerFrame();
        _device->vkTable().vkCmdResetQueryPool(
            commandBuffer.vkCommandBuffer(), _device->vkQueryPool(), frame * queriesPerFrame * 2, queriesPerFrame);
    }

    _indices[poolIndex] = (_indices[poolIndex] + 1) % _buffersPerFrame;
    return &commandBuffer;
}

CommandBuffer* CommandBufferPool::getSecondary(gerium_uint32_t frame,
                                               gerium_uint32_t thread,
                                               RenderPassHandle renderPass,
                                               FramebufferHandle framebuffer,
                                               bool profile) {
    const auto poolIndex   = getPoolIndex(frame, thread + 1);
    const auto bufferIndex = _indices[poolIndex];

    auto& commandBuffer = _commandBuffers[poolIndex * _buffersPerFrame + bufferIndex];

    if (commandBuffer.isRecording()) {
        throw Exception(GERIUM_RESULT_ERROR_INVALID_OPERATION);
    }

    commandBuffer.begin(renderPass, framebuffer);

    if (profile && _device->isProfilerEnable() && !_device->profiler()->hasTimestamps()) {
        const auto queriesPerFrame = _device->profiler()->queriesPerFrame();
        _device->vkTable().vkCmdResetQueryPool(
            commandBuffer.vkCommandBuffer(), _device->vkQueryPool(), frame * queriesPerFrame * 2, queriesPerFrame);
    }

    _indices[poolIndex] = (_indices[poolIndex] + 1) % _buffersPerFrame;
    return &commandBuffer;
}

gerium_uint32_t CommandBufferPool::getPoolIndex(gerium_uint32_t frame, gerium_uint32_t thread) const noexcept {
    return frame * _threadCount + thread;
}

} // namespace gerium::vulkan
