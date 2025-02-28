#include "Device.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace gerium::vulkan {

Device::~Device() {
    if (_device) {
        _vkTable.vkDeviceWaitIdle(_device);

        ImGui_ImplVulkan_Shutdown();
        _application->shutdownImGui();
        ImGui::DestroyContext();
        deleteResources(true);

        for (auto [_, view] : _unusedImageViews) {
            _vkTable.vkDestroyImageView(_device, view, getAllocCalls());
        }

        for (auto framebuffer : _framebuffers) {
            destroyFramebuffer(_framebuffers.handle(framebuffer));
        }
        deleteResources(true);

        for (auto pipeline : _pipelines) {
            destroyPipeline(_pipelines.handle(pipeline));
        }
        deleteResources(true);

        for (auto descriptorSet : _descriptorSets) {
            destroyDescriptorSet(_descriptorSets.handle(descriptorSet));
        }
        deleteResources(true);

        for (auto layout : _descriptorSetLayouts) {
            destroyDescriptorSetLayout(_descriptorSetLayouts.handle(layout));
        }
        deleteResources(true);

        for (auto program : _programs) {
            destroyProgram(_programs.handle(program));
        }
        deleteResources(true);

        for (auto renderPass : _renderPasses) {
            destroyRenderPass(_renderPasses.handle(renderPass));
        }
        deleteResources(true);

        for (auto texture : _textures) {
            destroyTexture(_textures.handle(texture));
        }
        deleteResources(true);

        for (auto sampler : _samplers) {
            destroySampler(_samplers.handle(sampler));
        }
        deleteResources(true);

        for (auto buffer : _buffers) {
            destroyBuffer(_buffers.handle(buffer));
        }
        deleteResources(true);

        if (_swapchain) {
            _vkTable.vkDestroySwapchainKHR(_device, _swapchain, getAllocCalls());
        }

        for (uint32_t i = 0; i < kMaxFrames; ++i) {
            _vkTable.vkDestroySemaphore(_device, _imageAvailableSemaphores[i], getAllocCalls());
            _vkTable.vkDestroySemaphore(_device, _renderFinishedSemaphores[i], getAllocCalls());
            _vkTable.vkDestroyFence(_device, _inFlightFences[i], getAllocCalls());
        }

        if (_vmaAllocator) {
            vmaDestroyAllocator(_vmaAllocator);
        }

        if (_imguiPool) {
            _vkTable.vkDestroyDescriptorPool(_device, _imguiPool, getAllocCalls());
        }

        for (auto pool : _descriptorPools) {
            if (pool) {
                _vkTable.vkDestroyDescriptorPool(_device, pool, getAllocCalls());
            }
        }

        if (_globalDescriptorPool) {
            _vkTable.vkDestroyDescriptorPool(_device, _globalDescriptorPool, getAllocCalls());
        }

        _profiler.reset();

        if (_queryPool) {
            _vkTable.vkDestroyQueryPool(_device, _queryPool, getAllocCalls());
        }

        _commandBufferPool.destroy();

        _vkTable.vkDestroyDevice(_device, getAllocCalls());
    }

    if (_surface) {
        _vkTable.vkDestroySurfaceKHR(_instance, _surface, getAllocCalls());
    }

    if (_instance) {
        _vkTable.vkDestroyInstance(_instance, getAllocCalls());
    }
}

void Device::create(Application* application,
                    gerium_feature_flags_t features,
                    gerium_uint32_t version,
                    bool enableValidations) {
    _enableValidations = enableValidations;
    _enableDebugNames  = enableValidations;
    _application       = application;
    _logger            = Logger::create("gerium:renderer:vulkan");
    _logger->setLevel(enableValidations ? GERIUM_LOGGER_LEVEL_DEBUG : GERIUM_LOGGER_LEVEL_OFF);
    _application->getSize(&_appWidth, &_appHeight);

    createInstance(application->getTitle(), version);
    createSurface(application);
    createPhysicalDevice();
    createDevice(application->workerThreadCount(), features);
    createProfiler(64);
    createDescriptorPools();
    createVmaAllocator();
    createDynamicBuffers();
    createDefaultSampler();
    createDefaultTexture();
    createSynchronizations();
    createSwapchain(application);
    createImGui(application);
    createFidelityFX();
}

bool Device::newFrame() {
    gerium_uint16_t width, height;
    _application->getSize(&width, &height);

    if (width == 0 || height == 0) {
        return false;
    }

    constexpr auto max = std::numeric_limits<uint64_t>::max();

    check(_vkTable.vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, max));
    check(_vkTable.vkResetFences(_device, 1, &_inFlightFences[_currentFrame]));

    const auto result = _vkTable.vkAcquireNextImageKHR(_device,
                                                       _swapchain,
                                                       std::numeric_limits<uint64_t>::max(),
                                                       _imageAvailableSemaphores[_currentFrame],
                                                       VK_NULL_HANDLE,
                                                       &_swapchainImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        if (onNeedPostAcquireResize()) {
            _application->getSize(&_appWidth, &_appHeight);
            resizeSwapchain();
        }
    } else {
        check(result);
    }

    if (_frameCommandBuffer) {
        _frameCommandBuffer->submit(QueueType::Graphics);
        _frameCommandBuffer = nullptr;
    }

    _dynamicUBOAllocatedSize  = _dynamicUBOSize * _currentFrame;
    _dynamicSSBOAllocatedSize = _dynamicSSBOSize * _currentFrame;

    if (_profilerEnabled) {
        _profiler->resetTimestamps();
    }

    ImGui_ImplVulkan_NewFrame();
    _application->newFrameImGui();
    ImGui::NewFrame();

    if (!_frameCommandBuffer) {
        _frameCommandBuffer = getPrimaryCommandBuffer(false);
    }

    check(_vkTable.vkResetDescriptorPool(_device, _descriptorPools[_currentFrame], {}));
    decltype(_freeDescriptorSetQueue) saveDescriptorSets{};
    std::vector<VkDescriptorSet> freeDescriptorSets{};
    saveDescriptorSets.reserve(_freeDescriptorSetQueue.size());
    freeDescriptorSets.reserve(_freeDescriptorSetQueue.size());
    for (const auto& [descriptorSet, frame] : _freeDescriptorSetQueue) {
        if (_absoluteFrame - frame >= kMaxFrames) {
            freeDescriptorSets.push_back(descriptorSet);
        } else {
            saveDescriptorSets.emplace_back(descriptorSet, frame);
        }
    }
    if (!freeDescriptorSets.empty()) {
        check(_vkTable.vkFreeDescriptorSets(
            _device, _globalDescriptorPool, (uint32_t) freeDescriptorSets.size(), freeDescriptorSets.data()));
    }
    _freeDescriptorSetQueue = std::move(saveDescriptorSets);

    auto unusedImageViews = std::move(_unusedImageViews);
    for (auto [frame, view] : unusedImageViews) {
        if (_absoluteFrame - frame >= 2) {
            _vkTable.vkDestroyImageView(_device, view, getAllocCalls());
        } else {
            _unusedImageViews.emplace_back(frame, view);
        }
    }
    return true;
}

void Device::submit(CommandBuffer* commandBuffer) {
    assert(_numQueuedCommandBuffers < sizeof(_queuedCommandBuffers) / sizeof(_queuedCommandBuffers[0]));
    _queuedCommandBuffers[_numQueuedCommandBuffers++] = commandBuffer;
}

void Device::present() {
    submit(_frameCommandBuffer);

    _frameCommandBuffer = nullptr;

    VkCommandBuffer enqueuedCommandBuffers[16];
    for (uint32_t i = 0; i < _numQueuedCommandBuffers; ++i) {
        auto commandBuffer = _queuedCommandBuffers[i];
        commandBuffer->end();

        enqueuedCommandBuffers[i] = commandBuffer->vkCommandBuffer();
    }

    VkSemaphore waitSemaphores[]      = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[]    = { _renderFinishedSemaphores[_currentFrame] };

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = _numQueuedCommandBuffers;
    submitInfo.pCommandBuffers      = enqueuedCommandBuffers;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;
    check(_vkTable.vkQueueSubmit(_queueGraphic, 1, &submitInfo, _inFlightFences[_currentFrame]));

    VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &_swapchain;
    presentInfo.pImageIndices      = &_swapchainImageIndex;
    presentInfo.pResults           = nullptr;

    const auto result = _vkTable.vkQueuePresentKHR(_queuePresent, &presentInfo);

    _numQueuedCommandBuffers = 0;

    for (const auto& [handle, mip] : _finishedLoadTextures) {
        finishLoadTexture(handle, mip, true);
    }
    _finishedLoadTextures.clear();

    if (_profilerEnabled) {
        _profiler->fetchDataFromGpu();
    }

    vmaSetCurrentFrameIndex(_vmaAllocator, _currentFrame);

    gerium_uint16_t appWidth, appHeight;
    _application->getSize(&appWidth, &appHeight);
    const auto resized = _appWidth != appWidth || _appHeight != appHeight;

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
        _appWidth  = appWidth;
        _appHeight = appHeight;
        resizeSwapchain();
    } else {
        check(result);
    }

    frameCountersAdvance();
    deleteResources();
}

BufferHandle Device::createBuffer(const BufferCreation& creation) {
    assert(creation.size && "It is impossible to create an empty buffer");

    auto [handle, buffer] = _buffers.obtain_and_access();

    buffer->vkUsageFlags = toVkBufferUsageFlags(creation.usageFlags);
    buffer->usage        = creation.usage;
    buffer->state        = ResourceState::Undefined;
    buffer->size         = creation.size;
    buffer->name         = intern(creation.name);
    buffer->parent       = Undefined;

    constexpr auto dynamicBufferFlags = GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_INDEX_BIT |
                                        GERIUM_BUFFER_USAGE_UNIFORM_BIT | GERIUM_BUFFER_USAGE_STORAGE_BIT |
                                        GERIUM_BUFFER_USAGE_INDIRECT_BIT;

    const bool useGlobalBuffer = gerium_uint32_t(creation.usageFlags & dynamicBufferFlags) != 0;
    if (creation.usage == ResourceUsageType::Dynamic && useGlobalBuffer) {
        const auto useSSBO = (creation.usageFlags & GERIUM_BUFFER_USAGE_UNIFORM_BIT) == 0;
        buffer->parent     = useSSBO ? _dynamicSSBO : _dynamicUBO;
        return handle;
    }

    const auto vkUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | buffer->vkUsageFlags;

    VmaAllocationCreateFlags vmaFlags;
    switch (creation.usage) {
        case ResourceUsageType::Immutable:
            if (creation.initialData) {
                vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                           VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
                           VMA_ALLOCATION_CREATE_MAPPED_BIT;
            } else {
                vmaFlags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
                if (creation.persistent) {
                    _logger->print(GERIUM_LOGGER_LEVEL_ERROR, "Unable to create a memory mapped immutable buffer");
                    error(GERIUM_RESULT_ERROR_INVALID_ARGUMENT);
                }
            }
            break;
        case ResourceUsageType::Staging:
            vmaFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
            if (creation.persistent) {
                vmaFlags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
            }
            break;
        default:
            assert(!"unreachable code");
            break;
    }

    VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferCreateInfo.size                  = creation.size;
    bufferCreateInfo.usage                 = vkUsage;
    bufferCreateInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    bufferCreateInfo.queueFamilyIndexCount = 0;
    bufferCreateInfo.pQueueFamilyIndices   = nullptr;

    VmaAllocationCreateInfo allocationCreateInfo;
    allocationCreateInfo.flags          = vmaFlags;
    allocationCreateInfo.usage          = VMA_MEMORY_USAGE_AUTO;
    allocationCreateInfo.requiredFlags  = 0;
    allocationCreateInfo.preferredFlags = 0;
    allocationCreateInfo.memoryTypeBits = 0;
    allocationCreateInfo.pool           = VK_NULL_HANDLE;
    allocationCreateInfo.pUserData      = VK_NULL_HANDLE;
    allocationCreateInfo.priority       = 0.0f;

    VmaAllocationInfo allocationInfo{};

    check(vmaCreateBuffer(_vmaAllocator,
                          &bufferCreateInfo,
                          &allocationCreateInfo,
                          &buffer->vkBuffer,
                          &buffer->vmaAllocation,
                          &allocationInfo));
    buffer->vkDeviceMemory = allocationInfo.deviceMemory;

    if (_enableDebugNames && buffer->name) {
        vmaSetAllocationName(_vmaAllocator, buffer->vmaAllocation, buffer->name);
    }

    setObjectName(VK_OBJECT_TYPE_BUFFER, (uint64_t) buffer->vkBuffer, buffer->name);

    if (creation.persistent) {
        buffer->mappedData = static_cast<uint8_t*>(allocationInfo.pMappedData);
    }

    if (creation.initialData || creation.hasFillValue) {
        VkMemoryPropertyFlags memPropFlags;
        vmaGetAllocationMemoryProperties(_vmaAllocator, buffer->vmaAllocation, &memPropFlags);

        auto fill = [&creation](void* data) {
            auto ptr = (gerium_uint32_t*) data;
            for (uint32_t i = 0; i < creation.size / 4; i++) {
                *ptr++ = creation.fillValue;
            }
        };

        if (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            if (buffer->mappedData) {
                if (creation.initialData) {
                    memcpy(buffer->mappedData, creation.initialData, (size_t) creation.size);
                } else {
                    fill(buffer->mappedData);
                }
            } else {
                void* data = mapBuffer(handle);
                if (creation.initialData) {
                    memcpy(data, creation.initialData, (size_t) creation.size);
                } else {
                    fill(data);
                }
                unmapBuffer(handle);
            }
        } else if (creation.initialData) {
            BufferCreation stagingCreation{};
            stagingCreation.set(GERIUM_BUFFER_USAGE_STORAGE_BIT, ResourceUsageType::Dynamic, buffer->size);
            auto stagingBuffer = createBuffer(stagingCreation);

            auto ptr = mapBuffer(stagingBuffer, 0, buffer->size);
            memcpy(ptr, creation.initialData, buffer->size);
            unmapBuffer(stagingBuffer);

            _frameCommandBuffer->copyBuffer(stagingBuffer, handle);
            destroyBuffer(stagingBuffer);
        } else {
            _frameCommandBuffer->fillBuffer(handle, 0, buffer->size, creation.fillValue);
        }
    }

    return handle;
}

TextureHandle Device::createTexture(const TextureCreation& creation) {
    auto [handle, texture] = _textures.obtain_and_access();

    texture->vkFormat      = toVkFormat(creation.format);
    texture->size          = calcTextureSize(creation.width, creation.height, creation.depth, creation.format);
    texture->width         = creation.width;
    texture->height        = creation.height;
    texture->depth         = creation.depth;
    texture->mipBase       = 0;
    texture->mipLevels     = creation.mipmaps;
    texture->layers        = creation.layers;
    texture->flags         = creation.flags;
    texture->type          = creation.type;
    texture->name          = intern(creation.name);
    texture->parentTexture = Undefined;
    texture->sampler       = Undefined;

    auto imageFlags =
        creation.type == GERIUM_TEXTURE_TYPE_CUBE ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : VkImageCreateFlags{};

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    if ((creation.flags & TextureFlags::Compute) == TextureFlags::Compute) {
        usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (hasDepthOrStencil(texture->vkFormat)) {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    } else {
        const auto renderTarget = (creation.flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget;
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        usage |= renderTarget ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
    }

    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.flags                 = imageFlags;
    imageInfo.imageType             = toVkImageType(creation.type);
    imageInfo.format                = texture->vkFormat;
    imageInfo.extent.width          = texture->width;
    imageInfo.extent.height         = texture->height;
    imageInfo.extent.depth          = texture->depth;
    imageInfo.mipLevels             = texture->mipLevels;
    imageInfo.arrayLayers           = texture->layers;
    imageInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage                 = usage;
    imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices   = nullptr;
    imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    if (creation.alias == Undefined) {
        VmaAllocationCreateInfo memoryInfo{};
        memoryInfo.flags          = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT;
        memoryInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        check(vmaCreateImage(
            _vmaAllocator, &imageInfo, &memoryInfo, &texture->vkImage, &texture->vmaAllocation, nullptr));

        if (_enableDebugNames && texture->name) {
            vmaSetAllocationName(_vmaAllocator, texture->vmaAllocation, texture->name);
        }
    } else {
        auto aliasTexture = _textures.access(creation.alias);
        check(vmaCreateAliasingImage(_vmaAllocator, aliasTexture->vmaAllocation, &imageInfo, &texture->vkImage));
    }

    setObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t) texture->vkImage, texture->name);

    TextureViewCreation vc{};
    vc.setType(texture->type).setMips(0, texture->mipLevels).setArray(0, texture->layers).setName(texture->name);
    vkCreateImageView(vc, handle);

    if (creation.initialData) {
        uploadTextureData(handle, creation.initialData);
    }

    return handle;
}

TextureHandle Device::createTextureView(const TextureViewCreation& creation) {
    auto [handle, texture] = _textures.obtain_and_access();

    auto parentTexture = _textures.access(creation.texture);

    texture->vkImage       = parentTexture->vkImage;
    texture->vkFormat      = parentTexture->vkFormat;
    texture->size          = parentTexture->size;
    texture->width         = parentTexture->width;
    texture->height        = parentTexture->height;
    texture->depth         = parentTexture->depth;
    texture->mipBase       = creation.mipBaseLevel;
    texture->mipLevels     = creation.mipLevelCount;
    texture->layers        = creation.arrayLayerCount;
    texture->loadedMips    = creation.mipLevelCount;
    texture->flags         = parentTexture->flags;
    texture->type          = creation.type;
    texture->name          = intern(creation.name);
    texture->parentTexture = creation.texture;
    texture->sampler       = parentTexture->sampler;

    _textures.addReference(creation.texture);
    parentTexture->loadedMips = parentTexture->mipLevels;

    if (parentTexture->sampler != Undefined) {
        _samplers.addReference(parentTexture->sampler);
    }

    for (size_t i = 0; i < std::size(texture->states); ++i) {
        texture->states[i] = parentTexture->states[i];
    }

    vkCreateImageView(creation, handle);
    return handle;
}

SamplerHandle Device::createSampler(const SamplerCreation& creation) {
    const auto key = calcSamplerHash(creation);

    if (auto it = _samplerCache.find(key); it != _samplerCache.end()) {
        _samplers.addReference(it->second);
        return it->second;
    }

    auto [handle, sampler] = _samplers.obtain_and_access();

    sampler->minFilter    = creation.minFilter;
    sampler->magFilter    = creation.magFilter;
    sampler->mipFilter    = creation.mipFilter;
    sampler->addressModeU = creation.addressModeU;
    sampler->addressModeV = creation.addressModeV;
    sampler->addressModeW = creation.addressModeW;
    sampler->name         = intern(creation.name);

    VkSamplerCreateInfo createInfo{ VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    createInfo.magFilter               = creation.magFilter;
    createInfo.minFilter               = creation.minFilter;
    createInfo.mipmapMode              = creation.mipFilter;
    createInfo.addressModeU            = creation.addressModeU;
    createInfo.addressModeV            = creation.addressModeV;
    createInfo.addressModeW            = creation.addressModeW;
    createInfo.mipLodBias              = 0.0f;
    createInfo.anisotropyEnable        = VK_FALSE;
    createInfo.maxAnisotropy           = 0.0f;
    createInfo.compareEnable           = VK_FALSE;
    createInfo.compareOp               = VK_COMPARE_OP_NEVER;
    createInfo.minLod                  = 0;
    createInfo.maxLod                  = 16;
    createInfo.borderColor             = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    createInfo.unnormalizedCoordinates = VK_FALSE;

    VkSamplerReductionModeCreateInfo reductionInfo{ VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO };
    if (creation.reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
        reductionInfo.reductionMode = creation.reductionMode;
        createInfo.pNext            = &reductionInfo;
    }

    check(_vkTable.vkCreateSampler(_device, &createInfo, getAllocCalls(), &sampler->vkSampler));

    setObjectName(VK_OBJECT_TYPE_SAMPLER, (uint64_t) sampler->vkSampler, sampler->name);

    _samplerCache[key] = handle;

    return handle;
}

RenderPassHandle Device::createRenderPass(const RenderPassCreation& creation) {
    const auto key = hash(creation.output);
    if (auto it = _renderPassCache.find(key); it != _renderPassCache.end()) {
        _renderPasses.addReference(it->second);
        return it->second;
    }

    auto [handle, renderPass] = _renderPasses.obtain_and_access();

    renderPass->output       = creation.output;
    renderPass->name         = intern(creation.name);
    renderPass->vkRenderPass = vkCreateRenderPass(renderPass->output, renderPass->name);

    _renderPassCache[key] = handle;
    return handle;
}

FramebufferHandle Device::createFramebuffer(const FramebufferCreation& creation) {
    auto [handle, framebuffer] = _framebuffers.obtain_and_access();

    _renderPasses.addReference(creation.renderPass);

    framebuffer->renderPass             = creation.renderPass;
    framebuffer->width                  = creation.width;
    framebuffer->height                 = creation.height;
    framebuffer->layers                 = creation.layers;
    framebuffer->scaleX                 = creation.scaleX;
    framebuffer->scaleY                 = creation.scaleY;
    framebuffer->depthStencilAttachment = creation.depthStencilTexture;
    framebuffer->resize                 = creation.resize;
    framebuffer->name                   = intern(creation.name);

    framebuffer->numColorAttachments = creation.numRenderTargets;
    for (gerium_uint32_t i = 0; i < creation.numRenderTargets; ++i) {
        framebuffer->colorAttachments[i] = creation.outputTextures[i];
    }

    VkImageView framebufferAttachments[kMaxImageOutputs + 1]{};
    uint32_t activeAttachments = 0;

    for (; activeAttachments < framebuffer->numColorAttachments; ++activeAttachments) {
        auto texture = _textures.access(framebuffer->colorAttachments[activeAttachments]);
        _textures.addReference(texture);

        framebufferAttachments[activeAttachments] = texture->vkImageView;
    }

    if (framebuffer->depthStencilAttachment != Undefined) {
        auto texture = _textures.access(framebuffer->depthStencilAttachment);
        _textures.addReference(texture);

        framebufferAttachments[activeAttachments++] = texture->vkImageView;
    }

    auto renderPass = _renderPasses.access(framebuffer->renderPass);

    VkFramebufferCreateInfo createInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    createInfo.renderPass      = renderPass->vkRenderPass;
    createInfo.attachmentCount = activeAttachments;
    createInfo.pAttachments    = framebufferAttachments;
    createInfo.width           = framebuffer->width;
    createInfo.height          = framebuffer->height;
    createInfo.layers          = framebuffer->layers;
    check(_vkTable.vkCreateFramebuffer(_device, &createInfo, getAllocCalls(), &framebuffer->vkFramebuffer));

    setObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t) framebuffer->vkFramebuffer, framebuffer->name);

    return handle;
}

DescriptorSetHandle Device::createDescriptorSet(const DescriptorSetCreation& creation) {
    auto [handle, descriptorSet] = _descriptorSets.obtain_and_access();

    descriptorSet->layout  = Undefined;
    descriptorSet->changed = 1;
    descriptorSet->global  = creation.global;

    return handle;
}

DescriptorSetLayoutHandle Device::createDescriptorSetLayout(const DescriptorSetLayoutCreation& creation) {
    auto [handle, descriptorSetLayout] = _descriptorSetLayouts.obtain_and_access();

    descriptorSetLayout->data                      = *creation.setLayout;
    descriptorSetLayout->data.createInfo.pBindings = descriptorSetLayout->data.bindings.data();
    if (_bindlessSupported) {
        descriptorSetLayout->data.bindlessInfo.pBindingFlags = descriptorSetLayout->data.bindlessFlags.data();
        descriptorSetLayout->data.createInfo.pNext           = &descriptorSetLayout->data.bindlessInfo;
    }

    check(_vkTable.vkCreateDescriptorSetLayout(
        _device, &descriptorSetLayout->data.createInfo, getAllocCalls(), &descriptorSetLayout->vkDescriptorSetLayout));
    return handle;
}

ProgramHandle Device::createProgram(const ProgramCreation& creation, bool saveSpirv) {
    auto [handle, program] = _programs.obtain_and_access();

    // VkPipelineShaderStageCreateInfo shaderStageInfo[kMaxShaderStages];
    program->name             = intern(creation.name);
    program->graphicsPipeline = true;

    std::map<uint32_t, std::set<uint32_t>> uniqueBindings;

    for (; program->activeShaders < creation.stagesCount; ++program->activeShaders) {
        const auto& stage = creation.stages[program->activeShaders];

        if (stage.type == GERIUM_SHADER_TYPE_COMPUTE) {
            program->graphicsPipeline = false;
        }

        auto lang = stage.lang;
        if (lang == GERIUM_SHADER_LANGUAGE_UNKNOWN) {
            if (ctre::search<"#version \\d+">((const char*) stage.data)) {
                lang = GERIUM_SHADER_LANGUAGE_GLSL;
            } else if (ctre::search<"(SV_\\w+)">((const char*) stage.data)) {
                lang = GERIUM_SHADER_LANGUAGE_HLSL;
            } else {
                error(GERIUM_RESULT_ERROR_DETECT_SHADER_LANGUAGE);
            }
        }

        const auto stageType = toVkShaderStage(stage.type);

        auto& spirv = program->spirv[program->activeShaders];

        VkShaderModuleCreateInfo shaderInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

        if (lang == GERIUM_SHADER_LANGUAGE_SPIRV) {
            shaderInfo.codeSize = stage.size;
            shaderInfo.pCode    = reinterpret_cast<const uint32_t*>(stage.data);

            if (saveSpirv) {
                spirv = std::vector(shaderInfo.pCode, shaderInfo.pCode + shaderInfo.codeSize / 4);
            }
        } else if (lang == GERIUM_SHADER_LANGUAGE_GLSL || lang == GERIUM_SHADER_LANGUAGE_HLSL) {
            spirv = compile(
                (const char*) stage.data, stage.size, lang, stageType, stage.name, stage.macro_count, stage.macros);

            shaderInfo.codeSize = spirv.size() * 4;
            shaderInfo.pCode    = spirv.data();
        } else {
            assert(!"unreachable code");
        }

        VkPipelineShaderStageCreateInfo& shaderStageInfo = program->shaderStageInfo[program->activeShaders];
        shaderStageInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.pName                            = stage.entry_point ? stage.entry_point : "main";
        shaderStageInfo.stage                            = stageType;
        check(_vkTable.vkCreateShaderModule(
            _device, &shaderInfo, getAllocCalls(), &program->shaderStageInfo[program->activeShaders].module));

        SpvReflectShaderModule module{};
        if (spvReflectCreateShaderModule(shaderInfo.codeSize, (void*) shaderInfo.pCode, &module) !=
            SPV_REFLECT_RESULT_SUCCESS) {
            error(GERIUM_RESULT_ERROR_PARSE_SPIRV);
        }

        uint32_t count = 0;
        if (spvReflectEnumerateDescriptorSets(&module, &count, NULL) != SPV_REFLECT_RESULT_SUCCESS) {
            error(GERIUM_RESULT_ERROR_PARSE_SPIRV);
        }

        std::vector<SpvReflectDescriptorSet*> sets(count);
        if (spvReflectEnumerateDescriptorSets(&module, &count, sets.data()) != SPV_REFLECT_RESULT_SUCCESS) {
            error(GERIUM_RESULT_ERROR_PARSE_SPIRV);
        }

        for (uint32_t set = 0; set < sets.size(); ++set) {
            const SpvReflectDescriptorSet& reflSet = *(sets[set]);

            DescriptorSetLayoutData& layout = program->descriptorSets[reflSet.set];
            auto& uniqueBinding             = uniqueBindings[reflSet.set];

            layout.hash = 0;
            for (uint32_t binding = 0; binding < reflSet.binding_count; ++binding) {
                const SpvReflectDescriptorBinding& reflBinding = *(reflSet.bindings[binding]);
                if (uniqueBinding.contains(reflBinding.binding)) {
                    auto it =
                        std::find_if(layout.bindings.begin(), layout.bindings.end(), [&reflBinding](const auto& item) {
                        return item.binding == reflBinding.binding;
                    });
                    if (it != layout.bindings.end()) {
                        auto& layoutBinding = *it;
                        layoutBinding.stageFlags |= static_cast<VkShaderStageFlagBits>(module.shader_stage);
                        layout.hash = hash(layoutBinding.stageFlags, layout.hash);

                        auto descriptorType  = static_cast<VkDescriptorType>(reflBinding.descriptor_type);
                        auto descriptorCount = 1;
                        if (descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                            descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                        } else if (descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                            descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                        }
                        for (uint32_t iDim = 0; iDim < reflBinding.array.dims_count; ++iDim) {
                            descriptorCount *= reflBinding.array.dims[iDim];
                        }

                        if (layoutBinding.descriptorType != descriptorType ||
                            layoutBinding.descriptorCount != descriptorCount) {
                            error(GERIUM_RESULT_ERROR_DESCRIPTOR);
                        }
                    }
                    continue;
                }
                layout.bindings.push_back({});
                layout.bindlessFlags.push_back({});
                VkDescriptorSetLayoutBinding& layoutBinding = layout.bindings.back();
                VkDescriptorBindingFlags& bindingFlags      = layout.bindlessFlags.back();
                layoutBinding.binding                       = reflBinding.binding;
                layoutBinding.descriptorType  = static_cast<VkDescriptorType>(reflBinding.descriptor_type);
                layoutBinding.descriptorCount = 1;
                if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                }
                if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                }
                for (uint32_t iDim = 0; iDim < reflBinding.array.dims_count; ++iDim) {
                    layoutBinding.descriptorCount *= reflBinding.array.dims[iDim];
                }
                if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                    layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                    if (reflBinding.type_description && reflBinding.type_description->op == SpvOpTypeRuntimeArray) {
                        layoutBinding.descriptorCount = kBindlessPoolElements;
                        bindingFlags =
                            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
                    }
                    if (reflBinding.image.dim == SpvDim3D) {
                        layout.default3DTextures.insert(layoutBinding.binding);
                    }
                }
                layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);

                uniqueBinding.insert(reflBinding.binding);

                layout.hash = hash(layoutBinding.binding, layout.hash);
                layout.hash = hash(layoutBinding.descriptorType, layout.hash);
                layout.hash = hash(layoutBinding.descriptorCount, layout.hash);
                layout.hash = hash(layoutBinding.stageFlags, layout.hash);
            }
            layout.setNumber               = reflSet.set;
            layout.createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout.createInfo.bindingCount = (uint32_t) layout.bindings.size();
            layout.createInfo.pBindings    = layout.bindings.data();
            if (_bindlessSupported) {
                layout.bindlessInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
                layout.bindlessInfo.bindingCount  = layout.bindlessFlags.size();
                layout.bindlessInfo.pBindingFlags = layout.bindlessFlags.data();
                layout.createInfo.pNext           = &layout.bindlessInfo;
                layout.createInfo.flags           = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
            }
        }

        setObjectName(VK_OBJECT_TYPE_SHADER_MODULE,
                      (uint64_t) program->shaderStageInfo[program->activeShaders].module,
                      stage.name);
    }

    return handle;
}

PipelineHandle Device::createPipeline(const PipelineCreation& creation) {
    auto pc = creation;
    std::vector<std::unique_ptr<gerium_uint8_t[]>> files;

    for (uint32_t i = 0; i < pc.program.stagesCount; ++i) {
        auto& stage = pc.program.stages[i];
        if (!stage.data) {
            auto fullpath = std::filesystem::path(stage.name);
            if (!fullpath.is_absolute()) {
                fullpath = std::filesystem::path(File::getAppDir()) / stage.name;
            }
            auto fullpathStr = fullpath.string();
            auto file        = File::open(fullpathStr.c_str(), true);
            auto data        = file->map();

            files.emplace_back(new gerium_uint8_t[file->getSize()]);

            stage.data = (gerium_cdata_t) files.back().get();
            stage.size = (gerium_uint32_t) file->getSize();

            memcpy((void*) stage.data, data, stage.size);
        }
    }

    auto [handle, pipeline] = _pipelines.obtain_and_access();

    const auto filename     = "pipeline-"s + std::to_string(calcPipelineHash(pc)) + ".cache"s;
    const auto cachePath    = std::filesystem::path(File::getCacheDir()) / filename;
    const auto cachePathStr = cachePath.string();

    auto cacheExists = File::existsFile(cachePathStr.c_str());
    ObjectPtr<File> cacheFile;
    const gerium_uint8_t* cacheData = nullptr;

    VkPipelineCacheCreateInfo cacheInfo{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

    ProgramCreation cacheProgram{};
    const ProgramCreation* programCreation = &pc.program;

    if (cacheExists) {
        /**
         * Cache format:
         *
         * uint32 - pipeline cache size
         * [...]  - pipeline cache data
         * uint8  - shader stage count
         * [
         *     uint32 - spirv size
         *     [...]  - spirv data
         * ]
         **/

        cacheFile = File::open(cachePathStr.c_str(), true);
        cacheData = (const gerium_uint8_t*) cacheFile->map();

        const auto pipelineSize = *((gerium_uint32_t*) cacheData);
        cacheData += 4;

        auto header = (const VkPipelineCacheHeaderVersionOne*) cacheData;

        if (pipelineSize >= sizeof(VkPipelineCacheHeaderVersionOne) && header->deviceID == _deviceProperties.deviceID &&
            header->vendorID == _deviceProperties.vendorID &&
            memcmp(header->pipelineCacheUUID, _deviceProperties.pipelineCacheUUID, VK_UUID_SIZE) == 0) {
            programCreation = &cacheProgram;

            cacheInfo.initialDataSize = pipelineSize;
            cacheInfo.pInitialData    = (const void*) cacheData;

            cacheData += pipelineSize;

            auto stages = *cacheData;
            cacheData += 1;

            for (gerium_uint8_t i = 0; i < stages; ++i) {
                auto spirvSize = *((gerium_uint32_t*) cacheData);
                cacheData += 4;

                auto type = (gerium_shader_type_t) *cacheData;
                ++cacheData;

                gerium_shader_t shader;
                shader.type        = type;
                shader.lang        = GERIUM_SHADER_LANGUAGE_SPIRV;
                shader.entry_point = nullptr;
                shader.name        = nullptr;
                shader.data        = (gerium_cdata_t) cacheData;
                shader.size        = spirvSize;
                cacheProgram.addStage(shader);

                cacheData += spirvSize;
            }

        } else {
            cacheExists = false;
            cacheData   = nullptr;
            cacheFile   = nullptr;
        }
    }

    VkPipelineCache pipelineCache;
    check(_vkTable.vkCreatePipelineCache(_device, &cacheInfo, getAllocCalls(), &pipelineCache));
    defer(_vkTable.vkDestroyPipelineCache(_device, pipelineCache, getAllocCalls()));

    auto programHandle = createProgram(*programCreation, !cacheExists);
    auto program       = _programs.access(programHandle);
    defer(destroyProgram(programHandle));

    VkDescriptorSetLayout vkLayouts[kMaxDescriptorSetLayouts];
    uint32_t numActiveLayouts = 0;

    std::vector<uint32_t> sets;
    sets.reserve(program->descriptorSets.size());

    for (const auto& [set, _] : program->descriptorSets) {
        sets.push_back(set);
    }
    std::sort(sets.begin(), sets.end());

    for (uint32_t index : sets) {
        DescriptorSetLayoutCreation lc;
        lc.setName(nullptr);
        lc.setLayout                                           = &program->descriptorSets[index];
        pipeline->descriptorSetLayoutHandles[numActiveLayouts] = createDescriptorSetLayout(lc);

        auto layout = _descriptorSetLayouts.access(pipeline->descriptorSetLayoutHandles[numActiveLayouts]);
        vkLayouts[numActiveLayouts++] = layout->vkDescriptorSetLayout;
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pipelineLayoutInfo.pSetLayouts    = vkLayouts;
    pipelineLayoutInfo.setLayoutCount = numActiveLayouts;

    VkPipelineLayout pipelineLayout;
    check(_vkTable.vkCreatePipelineLayout(_device, &pipelineLayoutInfo, getAllocCalls(), &pipelineLayout));

    pipeline->vkPipelineLayout = pipelineLayout;
    pipeline->numActiveLayouts = numActiveLayouts;

    if (program->graphicsPipeline) {
        RenderPassCreation rc{};
        rc.output             = pc.renderPass;
        rc.name               = pc.name;
        pipeline->renderPass  = createRenderPass(rc);
        pipeline->vkBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        VkVertexInputAttributeDescription vertexAttributes[kMaxVertexAttributes];
        VkVertexInputBindingDescription vertexBindings[kMaxVertexBindings];

        if (pc.vertexInput.numVertexAttributes) {
            for (uint32_t i = 0; i < pc.vertexInput.numVertexAttributes; ++i) {
                const auto& vertexAttribute = pc.vertexInput.vertexAttributes[i];

                vertexAttributes[i] = { vertexAttribute.location,
                                        vertexAttribute.binding,
                                        toVkFormat(vertexAttribute.format),
                                        vertexAttribute.offset };
            }
            vertexInput.vertexAttributeDescriptionCount = pc.vertexInput.numVertexAttributes;
            vertexInput.pVertexAttributeDescriptions    = vertexAttributes;
        } else {
            vertexInput.vertexAttributeDescriptionCount = 0;
            vertexInput.pVertexAttributeDescriptions    = nullptr;
        }

        if (pc.vertexInput.numVertexBindings) {
            vertexInput.vertexBindingDescriptionCount = pc.vertexInput.numVertexBindings;

            for (uint32_t i = 0; i < pc.vertexInput.numVertexBindings; ++i) {
                const auto& vertexBinding = pc.vertexInput.vertexBindings[i];

                VkVertexInputRate vertexRate = vertexBinding.input_rate == GERIUM_VERTEX_RATE_PER_VERTEX
                                                   ? VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX
                                                   : VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;

                vertexBindings[i] = { vertexBinding.binding, vertexBinding.stride, vertexRate };
            }
            vertexInput.pVertexBindingDescriptions = vertexBindings;
        } else {
            vertexInput.vertexBindingDescriptionCount = 0;
            vertexInput.pVertexBindingDescriptions    = nullptr;
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
        };
        inputAssembly.topology               = toVkPrimitiveTopology(pc.rasterization->primitive_topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = (float) _swapchainExtent.width;
        viewport.height     = (float) _swapchainExtent.height;
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor = {};
        scissor.offset   = { 0, 0 };
        scissor.extent   = { _swapchainExtent.width, _swapchainExtent.height };

        VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        rasterizer.depthClampEnable        = pc.rasterization->depth_clamp_enable;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = toVkPolygonMode(pc.rasterization->polygon_mode);
        rasterizer.lineWidth               = pc.rasterization->line_width;
        rasterizer.cullMode                = toVkCullMode(pc.rasterization->cull_mode);
        rasterizer.frontFace               = toVkFrontFace(pc.rasterization->front_face);
        rasterizer.depthBiasEnable         = pc.rasterization->depth_bias_enable;
        rasterizer.depthBiasConstantFactor = pc.rasterization->depth_bias_constant_factor;
        rasterizer.depthBiasClamp          = pc.rasterization->depth_bias_clamp;
        rasterizer.depthBiasSlopeFactor    = pc.rasterization->depth_bias_slope_factor;

        VkPipelineMultisampleStateCreateInfo multisampling{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        multisampling.sampleShadingEnable   = VK_FALSE;
        multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading      = 1.0f;
        multisampling.pSampleMask           = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable      = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depthStencil{
            VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO
        };
        depthStencil.depthTestEnable       = pc.depthStencil->depth_test_enable;
        depthStencil.depthWriteEnable      = pc.depthStencil->depth_write_enable;
        depthStencil.depthCompareOp        = toVkCompareOp(pc.depthStencil->depth_compare_op);
        depthStencil.depthBoundsTestEnable = pc.depthStencil->depth_bounds_test_enable;
        depthStencil.stencilTestEnable     = pc.depthStencil->stencil_test_enable;
        depthStencil.front.failOp          = toVkStencilOp(pc.depthStencil->front.fail_op);
        depthStencil.front.passOp          = toVkStencilOp(pc.depthStencil->front.pass_op);
        depthStencil.front.depthFailOp     = toVkStencilOp(pc.depthStencil->front.depth_fail_op);
        depthStencil.front.compareOp       = toVkCompareOp(pc.depthStencil->front.compare_op);
        depthStencil.front.compareMask     = pc.depthStencil->front.compare_mask;
        depthStencil.front.writeMask       = pc.depthStencil->front.write_mask;
        depthStencil.front.reference       = pc.depthStencil->front.reference;
        depthStencil.back.failOp           = toVkStencilOp(pc.depthStencil->back.fail_op);
        depthStencil.back.passOp           = toVkStencilOp(pc.depthStencil->back.pass_op);
        depthStencil.back.depthFailOp      = toVkStencilOp(pc.depthStencil->back.depth_fail_op);
        depthStencil.back.compareOp        = toVkCompareOp(pc.depthStencil->back.compare_op);
        depthStencil.back.compareMask      = pc.depthStencil->back.compare_mask;
        depthStencil.back.writeMask        = pc.depthStencil->back.write_mask;
        depthStencil.back.reference        = pc.depthStencil->back.reference;
        depthStencil.minDepthBounds        = pc.depthStencil->min_depth_bounds;
        depthStencil.maxDepthBounds        = pc.depthStencil->max_depth_bounds;

        VkPipelineColorBlendAttachmentState colorBlendAttachment[kMaxImageOutputs];
        if (pc.blendState.activeStates) {
            assert(pc.blendState.activeStates == pc.renderPass.numColorFormats);
            for (uint32_t i = 0; i < pc.blendState.activeStates; i++) {
                const auto& writeMask  = pc.blendState.writeMasks[i];
                const auto& blendState = pc.blendState.blendStates[i];

                colorBlendAttachment[i].blendEnable         = blendState.blend_enable;
                colorBlendAttachment[i].srcColorBlendFactor = toVkBlendFactor(blendState.src_color_blend_factor);
                colorBlendAttachment[i].dstColorBlendFactor = toVkBlendFactor(blendState.dst_color_blend_factor);
                colorBlendAttachment[i].colorBlendOp        = toVkBlendOp(blendState.color_blend_op);
                colorBlendAttachment[i].srcAlphaBlendFactor = toVkBlendFactor(blendState.src_alpha_blend_factor);
                colorBlendAttachment[i].dstAlphaBlendFactor = toVkBlendFactor(blendState.dst_alpha_blend_factor);
                colorBlendAttachment[i].alphaBlendOp        = toVkBlendOp(blendState.alpha_blend_op);
                colorBlendAttachment[i].colorWriteMask      = toVkColorComponent(writeMask);
            }
        } else {
            for (uint32_t i = 0; i < pc.renderPass.numColorFormats; ++i) {
                colorBlendAttachment[i]                = {};
                colorBlendAttachment[i].blendEnable    = VK_FALSE;
                colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlending.logicOpEnable = pc.colorBlend->logic_op_enable;
        colorBlending.logicOp       = toVkLogicOp(pc.colorBlend->logic_op);
        colorBlending.attachmentCount =
            pc.blendState.activeStates ? pc.blendState.activeStates : pc.renderPass.numColorFormats;
        colorBlending.pAttachments      = colorBlendAttachment;
        colorBlending.blendConstants[0] = pc.colorBlend->blend_constants[0];
        colorBlending.blendConstants[1] = pc.colorBlend->blend_constants[1];
        colorBlending.blendConstants[2] = pc.colorBlend->blend_constants[2];
        colorBlending.blendConstants[3] = pc.colorBlend->blend_constants[3];

        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

        VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        dynamicState.dynamicStateCount = sizeof(dynamicStates) / sizeof(dynamicStates[0]);
        dynamicState.pDynamicStates    = dynamicStates;

        VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        pipelineInfo.stageCount          = program->activeShaders;
        pipelineInfo.pStages             = program->shaderStageInfo;
        pipelineInfo.pVertexInputState   = &vertexInput;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState      = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState   = &multisampling;
        pipelineInfo.pDepthStencilState  = &depthStencil;
        pipelineInfo.pColorBlendState    = &colorBlending;
        pipelineInfo.pDynamicState       = &dynamicState;
        pipelineInfo.layout              = pipelineLayout;
        pipelineInfo.renderPass          = _renderPasses.access(pipeline->renderPass)->vkRenderPass;

        check(_vkTable.vkCreateGraphicsPipelines(
            _device, pipelineCache, 1, &pipelineInfo, getAllocCalls(), &pipeline->vkPipeline));

    } else {
        pipeline->renderPass  = Undefined;
        pipeline->vkBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;

        VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        pipelineInfo.stage  = program->shaderStageInfo[0];
        pipelineInfo.layout = pipelineLayout;

        check(_vkTable.vkCreateComputePipelines(
            _device, pipelineCache, 1, &pipelineInfo, getAllocCalls(), &pipeline->vkPipeline));
    }

    if (!cacheExists) {
        gerium_uint32_t totalCacheSize = 4;

        size_t cacheSize = 0;
        check(_vkTable.vkGetPipelineCacheData(_device, pipelineCache, &cacheSize, nullptr));
        totalCacheSize += (gerium_uint32_t) cacheSize;

        const auto stages = (gerium_uint8_t) program->activeShaders;
        totalCacheSize += 1;

        gerium_uint32_t stageSizes[kMaxShaderStages]{};

        for (gerium_uint8_t i = 0; i < stages; ++i) {
            stageSizes[i] = gerium_uint32_t(program->spirv[i].size() * 4);
            totalCacheSize += 5 + stageSizes[i];
        }

        auto saveCache = File::create(cachePathStr.c_str(), totalCacheSize);
        auto data      = (gerium_uint8_t*) saveCache->map();

        *(gerium_uint32_t*) (data) = (gerium_uint32_t) cacheSize;
        data += 4;
        check(_vkTable.vkGetPipelineCacheData(_device, pipelineCache, &cacheSize, (void*) data));

        data += cacheSize;
        *data = stages;
        ++data;

        for (gerium_uint8_t i = 0; i < stages; ++i) {
            *(gerium_uint32_t*) (data) = stageSizes[i];
            data += 4;

            *data = (gerium_uint8_t) (toGerumShaderType(program->shaderStageInfo[i].stage));
            ++data;

            memcpy((void*) data, (const void*) program->spirv[i].data(), stageSizes[i]);
            data += stageSizes[i];
        }
    }

    return handle;
}

void Device::destroyBuffer(BufferHandle handle) {
    _deletionQueue.push({ ResourceType::Buffer, _currentFrame, handle });
}

void Device::destroyTexture(TextureHandle handle) {
    _deletionQueue.push({ ResourceType::Texture, _currentFrame, handle });
}

void Device::destroySampler(SamplerHandle handle) {
    _deletionQueue.push({ ResourceType::Sampler, _currentFrame, handle });
}

void Device::destroyRenderPass(RenderPassHandle handle) {
    _deletionQueue.push({ ResourceType::RenderPass, _currentFrame, handle });
}

void Device::destroyFramebuffer(FramebufferHandle handle) {
    _deletionQueue.push({ ResourceType::Framebuffer, _currentFrame, handle });
}

void Device::destroyDescriptorSet(DescriptorSetHandle handle) {
    _deletionQueue.push({ ResourceType::DescriptorSet, _currentFrame, handle });
}

void Device::destroyDescriptorSetLayout(DescriptorSetLayoutHandle handle) {
    _deletionQueue.push({ ResourceType::DescriptorSetLayout, _currentFrame, handle });
}

void Device::destroyProgram(ProgramHandle handle) {
    _deletionQueue.push({ ResourceType::Program, _currentFrame, handle });
}

void Device::destroyPipeline(PipelineHandle handle) {
    _deletionQueue.push({ ResourceType::Pipeline, _currentFrame, handle });
}

void* Device::mapBuffer(BufferHandle handle, uint32_t offset, uint32_t size) {
    auto buffer = _buffers.access(handle);

    size = align(size ? size : buffer->size, _alignment);

    if (buffer->parent == _dynamicUBO) {
        buffer->globalOffset = _dynamicUBOAllocatedSize;
        buffer->mappedOffset = _dynamicUBOAllocatedSize + offset;
        buffer->mappedSize   = size;

        uint8_t* mappedMemory = _dynamicUBOMapped + buffer->mappedOffset;
        _dynamicUBOAllocatedSize += size;

        assert(_dynamicUBOAllocatedSize < _dynamicUBOSize * (_currentFrame + 1));

        return (void*) mappedMemory;
    } else if (buffer->parent == _dynamicSSBO) {
        buffer->globalOffset = _dynamicSSBOAllocatedSize;
        buffer->mappedOffset = _dynamicSSBOAllocatedSize + offset;
        buffer->mappedSize   = size;

        uint8_t* mappedMemory = _dynamicSSBOMapped + buffer->mappedOffset;
        _dynamicSSBOAllocatedSize += size;

        assert(_dynamicSSBOAllocatedSize < _dynamicSSBOSize * (_currentFrame + 1));

        return (void*) mappedMemory;
    }

    void* data = nullptr;
    check(vmaMapMemory(_vmaAllocator, buffer->vmaAllocation, &data));

    buffer->mappedOffset = offset;
    buffer->mappedSize   = size;

    return (uint8_t*) data + offset;
}

void Device::unmapBuffer(BufferHandle handle) {
    auto buffer = _buffers.access(handle);

    if (buffer->parent != Undefined) {
        auto parentBuffer = _buffers.access(buffer->parent);
        vmaFlushAllocation(_vmaAllocator, parentBuffer->vmaAllocation, buffer->mappedOffset, buffer->mappedSize);
        return;
    }

    if (buffer->mappedData) {
        vmaFlushAllocation(_vmaAllocator, buffer->vmaAllocation, buffer->mappedOffset, buffer->mappedSize);
    }

    vmaUnmapMemory(_vmaAllocator, buffer->vmaAllocation);
}

void Device::finishLoadTexture(TextureHandle handle, uint8_t mip, bool immediately) {
    if (immediately) {
        auto texture        = _textures.access(handle);
        texture->loadedMips = texture->mipLevels - mip;
    } else {
        _finishedLoadTextures.emplace_back(handle, mip);
    }
}

void Device::showViewMips(TextureHandle handle, uint8_t mip) {
    auto texture = _textures.access(handle);
    TextureViewCreation vc{};
    vc.setType(texture->type)
        .setMips(mip, texture->mipLevels - mip)
        .setArray(0, texture->layers)
        .setName(texture->name);
    _unusedImageViews.emplace_back(_absoluteFrame, texture->vkImageView);
    vkCreateImageView(vc, handle);
}

void Device::bind(DescriptorSetHandle handle,
                  gerium_uint16_t binding,
                  gerium_uint16_t element,
                  Handle resource,
                  bool dynamic,
                  gerium_utf8_t resourceInput,
                  bool fromPreviousFrame) {
    auto descriptorSet       = _descriptorSets.access(handle);
    auto internResourceInput = intern(resourceInput);

    const auto key = calcBindingKey(binding, element);

    if (_bindlessSupported && descriptorSet->global && descriptorSet->layout != Undefined) {
        auto layout = _descriptorSetLayouts.access(descriptorSet->layout);
        auto it = std::find_if(layout->data.bindings.cbegin(), layout->data.bindings.cend(), [binding](const auto& b) {
            return b.binding == binding;
        });
        if (it != layout->data.bindings.cend()) {
            const auto index      = it - layout->data.bindings.cbegin();
            const auto isBindless = (layout->data.bindlessFlags[index] & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) ==
                                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
            if (isBindless) {
                DescriptorSet tempDescriptorSet{};
                tempDescriptorSet.vkDescriptorSet = descriptorSet->vkDescriptorSet;

                auto& item         = tempDescriptorSet.bindings[key];
                item.binding       = binding;
                item.element       = element;
                item.resource      = internResourceInput;
                item.previousFrame = fromPreviousFrame;
                item.handle        = resource;
                VkWriteDescriptorSet descriptorWrite[1]{};
                VkDescriptorBufferInfo bufferInfo[1]{};
                VkDescriptorImageInfo imageInfo[1]{};
                const auto [num, _] =
                    fillWriteDescriptorSets(*layout, tempDescriptorSet, descriptorWrite, bufferInfo, imageInfo);

                _vkTable.vkUpdateDescriptorSets(_device, num, descriptorWrite, 0, nullptr);
                return;
            }
        }
    }

    if (auto it = descriptorSet->bindings.find(key); it != descriptorSet->bindings.end()) {
        if (it->second.binding != binding || it->second.element != element ||
            it->second.resource != internResourceInput || it->second.handle != resource ||
            it->second.previousFrame != fromPreviousFrame) {
            it->second.binding       = binding;
            it->second.element       = element;
            it->second.resource      = internResourceInput;
            it->second.previousFrame = fromPreviousFrame;
            it->second.handle        = resource;

            if (!descriptorSet->changed) {
                descriptorSet->changed = !dynamic;
            }
        }
    } else {
        auto& item         = descriptorSet->bindings[key];
        item.binding       = binding;
        item.element       = element;
        item.resource      = internResourceInput;
        item.previousFrame = fromPreviousFrame;
        item.handle        = resource;

        descriptorSet->changed = true;
    }
}

VkDescriptorSet Device::updateDescriptorSet(DescriptorSetHandle handle,
                                            DescriptorSetLayoutHandle layoutHandle,
                                            FrameGraph* frameGraph) {
    auto descriptorSet = _descriptorSets.access(handle);

    marl::lock lock(_descriptorPoolMutex);
    auto recreate = descriptorSet->changed || descriptorSet->layout != layoutHandle ||
                    (!descriptorSet->global && descriptorSet->absoluteFrame != _absoluteFrame);

    if (recreate) {
        auto pipelineLayout = _descriptorSetLayouts.access(layoutHandle);

        bool swapToPrevResource = false;
        for (auto& [_, item] : descriptorSet->bindings) {
            if (item.resource) {
                auto resourceHandle = findInputResource(item.resource, item.previousFrame);
                bind(handle, item.binding, item.element, resourceHandle, false, item.resource, item.previousFrame);
            }
        }

        bool bindless = false;
        if (_bindlessSupported) {
            for (auto flags : pipelineLayout->data.bindlessFlags) {
                if ((flags & VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) == VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT) {
                    bindless = true;
                    break;
                }
            }
        }

        if (descriptorSet->vkDescriptorSet && descriptorSet->global) {
            _freeDescriptorSetQueue.emplace_back(descriptorSet->vkDescriptorSet, _absoluteFrame);
        }
        uint32_t maxBinding = kBindlessPoolElements - 1;

        VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO
        };
        countInfo.descriptorSetCount = 1;
        countInfo.pDescriptorCounts  = &maxBinding;

        VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocInfo.pNext              = _bindlessSupported && bindless ? &countInfo : nullptr;
        allocInfo.descriptorPool     = descriptorSet->global ? _globalDescriptorPool : _descriptorPools[_currentFrame];
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts        = &pipelineLayout->vkDescriptorSetLayout;

        check(_vkTable.vkAllocateDescriptorSets(_device, &allocInfo, &descriptorSet->vkDescriptorSet));

        const auto [num, updateRequired] =
            fillWriteDescriptorSets(*pipelineLayout, *descriptorSet, _descriptorWrite, _bufferInfo, _imageInfo);

        _vkTable.vkUpdateDescriptorSets(_device, num, _descriptorWrite, 0, nullptr);

        descriptorSet->layout  = layoutHandle;
        descriptorSet->changed = (updateRequired || swapToPrevResource) && !bindless;
    }

    descriptorSet->absoluteFrame = _absoluteFrame;
    return descriptorSet->vkDescriptorSet;
}

CommandBuffer* Device::getPrimaryCommandBuffer(bool profile) {
    return _commandBufferPool.getPrimary(_currentFrame, profile);
}

CommandBuffer* Device::getSecondaryCommandBuffer(gerium_uint32_t thread,
                                                 RenderPassHandle renderPass,
                                                 FramebufferHandle framebuffer) {
    return _commandBufferPool.getSecondary(_currentFrame, thread, renderPass, framebuffer, false);
}

SamplerHandle Device::getTextureSampler(TextureHandle texture) const noexcept {
    return _textures.access(texture)->sampler;
}

void Device::linkTextureSampler(TextureHandle texture, SamplerHandle sampler) noexcept {
    _textures.access(texture)->sampler = sampler;
}

FfxInterface Device::createFfxInterface(gerium_uint32_t maxContexts) {
    if (!_fidelityFXSupported) {
        error(GERIUM_RESULT_ERROR_FIDELITY_FX_NOT_SUPPORTED);
    }

    const size_t scratchBufferSize = ffxGetScratchMemorySizeVK(&_ffxDeviceContext, maxContexts);
    auto scratchBuffer             = new uint8_t[scratchBufferSize];
    memset(scratchBuffer, 0, scratchBufferSize);

    FfxInterface ffxInterface{};
    ffxGetInterfaceVK(&ffxInterface, &_ffxDeviceContext, scratchBuffer, scratchBufferSize, maxContexts);
    return ffxInterface;
}

void Device::destroyFfxInterface(FfxInterface* ffxInterface) {
    delete[] ((uint8_t*) ffxInterface->scratchBuffer);
}

void Device::waitFfxJobs() const noexcept {
    _vkTable.vkDeviceWaitIdle(_device);
}

void Device::clearInputResources() {
    _currentInputResources.clear();
}

void Device::addInputResource(const FrameGraphResource* resource, Handle handle, bool fromPreviousFrame) {
    const auto key              = calcKeyInputResource(resource->name, fromPreviousFrame);
    _currentInputResources[key] = handle;
}

Handle Device::findInputResource(gerium_utf8_t resource, bool fromPreviousFrame) const noexcept {
    const auto key = calcKeyInputResource(resource, fromPreviousFrame);
    if (auto it = _currentInputResources.find(key); it != _currentInputResources.end()) {
        return it->second;
    }
    return Undefined;
}

gerium_uint64_t Device::calcKeyInputResource(gerium_utf8_t resource, bool fromPreviousFrame) noexcept {
    auto key = hash(resource);
    return hash(fromPreviousFrame, key);
}

bool Device::isSupportedFormat(gerium_format_t format) noexcept {
    const auto vkFormat = toVkFormat(format);

    VkFormatProperties properties;
    _vkTable.vkGetPhysicalDeviceFormatProperties(_physicalDevice, vkFormat, &properties);

    if (hasDepthOrStencil(vkFormat)) {
        constexpr auto flags = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        return (properties.optimalTilingFeatures & flags) == flags;
    } else {
        constexpr auto flags = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT | VK_FORMAT_FEATURE_BLIT_SRC_BIT |
                               VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT |
                               VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
        return (properties.optimalTilingFeatures & flags) == flags;
    }
}

uint32_t Device::totalMemoryUsed() {
    if (_vmaBudget.size() != _deviceMemProperties.memoryHeapCount) {
        _vmaBudget.resize(_deviceMemProperties.memoryHeapCount);
    }
    vmaGetHeapBudgets(_vmaAllocator, _vmaBudget.data());

    uint32_t total = 0;
    for (const auto& budget : _vmaBudget) {
        total += budget.usage;
    }
    return total;
}

void Device::createInstance(gerium_utf8_t appName, gerium_uint32_t version) {
#if VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL != 0
    _vkTable.init();
#else
    _vkTable.init(vkGetInstanceProcAddr);
#endif

    auto major      = (version >> 16) & 0xFF;
    auto minor      = (version >> 8) & 0xFF;
    auto micro      = (version >> 0) & 0xFF;
    auto appVersion = VK_MAKE_API_VERSION(0, major, minor, micro);

    printValidationLayers();
    printExtensions();

    const auto layers     = selectValidationLayers();
    const auto extensions = selectExtensions();

    if (layers.empty() || !contains(extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        _enableDebugNames = false;
    }

    constexpr auto messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    constexpr auto messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
    constexpr std::array validationFeatures = { VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
                                                VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT };

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pNext              = nullptr;
    appInfo.pApplicationName   = appName;
    appInfo.applicationVersion = appVersion;
    appInfo.pEngineName        = "gerium";
    appInfo.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    VkDebugUtilsMessengerCreateInfoEXT debugInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    debugInfo.pNext           = nullptr;
    debugInfo.flags           = 0;
    debugInfo.messageSeverity = messageSeverity;
    debugInfo.messageType     = messageType;
    debugInfo.pfnUserCallback = debugUtilsMessengerCallback;
    debugInfo.pUserData       = (void*) this;

    VkValidationFeaturesEXT features{ VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT };
    features.pNext                          = &debugInfo;
    features.enabledValidationFeatureCount  = validationFeatures.size();
    features.pEnabledValidationFeatures     = validationFeatures.data();
    features.disabledValidationFeatureCount = 0;
    features.pDisabledValidationFeatures    = nullptr;

    VkInstanceCreateInfo createInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    createInfo.pNext                   = onGetNextCreateInfo(layers.empty() ? nullptr : &features);
    createInfo.flags                   = onGetCreateInfoFlags();
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = (uint32_t) layers.size();
    createInfo.ppEnabledLayerNames     = layers.data();
    createInfo.enabledExtensionCount   = (uint32_t) extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    check(_vkTable.vkCreateInstance(&createInfo, getAllocCalls(), &_instance));

    _vkTable.init(_instance, _vkTable.vkGetInstanceProcAddr);
}

void Device::createSurface(Application* application) {
    _surface = onCreateSurface(application);
}

void Device::createPhysicalDevice() {
    printPhysicalDevices();
    _physicalDevice = selectPhysicalDevice();
    _queueFamilies  = getQueueFamilies(_physicalDevice);

    _vkTable.vkGetPhysicalDeviceProperties(_physicalDevice, &_deviceProperties);
    _vkTable.vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_deviceMemProperties);
    _alignment = (uint32_t) std::max(_deviceProperties.limits.minUniformBufferOffsetAlignment,
                                     _deviceProperties.limits.minStorageBufferOffsetAlignment);

    _profilerSupported =
        _deviceProperties.limits.timestampPeriod != 0 && _deviceProperties.limits.timestampComputeAndGraphics;
    if (_profilerSupported) {
        _profilerSupported = _queueFamilies.graphic.value().timestampValidBits != 0 &&
                             _queueFamilies.compute.value().timestampValidBits != 0;
    }
    if (_profilerSupported) {
        _profilerEnabled = _enableValidations;
    }
}

void Device::createDevice(gerium_uint32_t threadCount, gerium_feature_flags_t featureFlags) {
    const float priorities[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const auto layers        = selectValidationLayers();
    const auto extensions    = selectDeviceExtensions(_physicalDevice, featureFlags & GERIUM_FEATURE_MESH_SHADER_BIT);

    _memoryBudgetSupported = contains(extensions, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME);

    _fidelityFXSupported = contains(extensions, VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);

    _meshShaderSupported = (featureFlags & GERIUM_FEATURE_MESH_SHADER_BIT) == GERIUM_FEATURE_MESH_SHADER_BIT &&
                           contains(extensions, VK_EXT_MESH_SHADER_EXTENSION_NAME);

    _8BitStorageSupported = (featureFlags & GERIUM_FEATURE_8_BIT_STORAGE_BIT) == GERIUM_FEATURE_8_BIT_STORAGE_BIT;

    size_t queueCreateInfoCount                 = 0;
    VkDeviceQueueCreateInfo queueCreateInfos[4] = {};

    const auto& graphic  = _queueFamilies.graphic.value();
    const auto& compute  = _queueFamilies.compute.value();
    const auto& present  = _queueFamilies.present.value();
    const auto& transfer = _queueFamilies.transfer.value();

    std::set<uint32_t> uniqueQueueIndices = {
        graphic.index,
        compute.index,
        present.index,
        transfer.index,
    };

    std::map<uint32_t, uint32_t> queueCounts{};
    queueCounts[graphic.index]  = std::max(queueCounts[graphic.index], uint32_t(graphic.queue) + 1);
    queueCounts[compute.index]  = std::max(queueCounts[compute.index], uint32_t(compute.queue) + 1);
    queueCounts[present.index]  = std::max(queueCounts[present.index], uint32_t(present.queue) + 1);
    queueCounts[transfer.index] = std::max(queueCounts[transfer.index], uint32_t(transfer.queue) + 1);

    for (const auto& index : uniqueQueueIndices) {
        auto& info            = queueCreateInfos[queueCreateInfoCount++];
        info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.pNext            = nullptr;
        info.flags            = 0;
        info.queueFamilyIndex = index;
        info.queueCount       = queueCounts[index];
        info.pQueuePriorities = priorities;
    }

    void* pNext = nullptr;

    VkPhysicalDeviceVulkan11Features testFeatures11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
    pNext = &testFeatures11;

    VkPhysicalDeviceVulkan12Features testFeatures12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    testFeatures12.pNext = pNext;
    pNext                = &testFeatures12;

    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT
    };
    if (_meshShaderSupported) {
        meshShaderFeatures.pNext = pNext;
        pNext                    = &meshShaderFeatures;
    }

    VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, pNext };
    _vkTable.vkGetPhysicalDeviceFeatures2(_physicalDevice, &deviceFeatures);

    _bindlessSupported = (featureFlags & GERIUM_FEATURE_BINDLESS_BIT) == GERIUM_FEATURE_BINDLESS_BIT &&
                         testFeatures12.shaderSampledImageArrayNonUniformIndexing &&
                         testFeatures12.descriptorBindingPartiallyBound && testFeatures12.runtimeDescriptorArray &&
                         testFeatures12.descriptorBindingSampledImageUpdateAfterBind;
    _fidelityFXSupported = _fidelityFXSupported && testFeatures12.shaderStorageBufferArrayNonUniformIndexing &&
                           testFeatures12.shaderFloat16 && deviceFeatures.features.shaderImageGatherExtended;
    _meshShaderSupported  = _meshShaderSupported && meshShaderFeatures.meshShader && meshShaderFeatures.taskShader;
    _8BitStorageSupported = _8BitStorageSupported && testFeatures12.storageBuffer8BitAccess &&
                            testFeatures12.uniformAndStorageBuffer8BitAccess && testFeatures12.shaderInt8;
    _16BitStorageSupported = (featureFlags & GERIUM_FEATURE_16_BIT_STORAGE_BIT) == GERIUM_FEATURE_16_BIT_STORAGE_BIT &&
                             testFeatures11.storageBuffer16BitAccess &&
                             testFeatures11.uniformAndStorageBuffer16BitAccess && deviceFeatures.features.shaderInt16;

    meshShaderFeatures.pNext = nullptr;

    VkPhysicalDeviceVulkan11Features features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
    features11.shaderDrawParameters = VK_TRUE;
    if (_16BitStorageSupported) {
        features11.storageBuffer16BitAccess           = testFeatures11.storageBuffer16BitAccess;
        features11.uniformAndStorageBuffer16BitAccess = testFeatures11.uniformAndStorageBuffer16BitAccess;
    }

    VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    features12.pNext = &features11;

    VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    features.pNext = &features12;

    _samplerFilterMinmaxSupported =
        (featureFlags & GERIUM_FEATURE_SAMPLER_FILTER_MINMAX_BIT) == GERIUM_FEATURE_SAMPLER_FILTER_MINMAX_BIT
            ? testFeatures12.samplerFilterMinmax
            : false;

    features12.samplerFilterMinmax = _samplerFilterMinmaxSupported ? VK_TRUE : VK_FALSE;
    features12.drawIndirectCount   = testFeatures12.drawIndirectCount;
    if (_bindlessSupported) {
        features12.shaderSampledImageArrayNonUniformIndexing = testFeatures12.shaderSampledImageArrayNonUniformIndexing;
        features12.descriptorBindingPartiallyBound           = testFeatures12.descriptorBindingPartiallyBound;
        features12.runtimeDescriptorArray                    = testFeatures12.runtimeDescriptorArray;
        features12.descriptorBindingSampledImageUpdateAfterBind =
            testFeatures12.descriptorBindingSampledImageUpdateAfterBind;
    }
    if (_fidelityFXSupported) {
        features.features.shaderImageGatherExtended = deviceFeatures.features.shaderImageGatherExtended;
        features12.shaderFloat16                    = testFeatures12.shaderFloat16;
        features12.shaderStorageBufferArrayNonUniformIndexing =
            testFeatures12.shaderStorageBufferArrayNonUniformIndexing;
    }
    if (_8BitStorageSupported) {
        features12.storageBuffer8BitAccess           = testFeatures12.storageBuffer8BitAccess;
        features12.uniformAndStorageBuffer8BitAccess = testFeatures12.uniformAndStorageBuffer8BitAccess;
        features12.shaderInt8                        = testFeatures12.shaderInt8;
    }

    features.features.imageCubeArray             = deviceFeatures.features.imageCubeArray;
    features.features.geometryShader             = deviceFeatures.features.geometryShader;
    features.features.logicOp                    = deviceFeatures.features.logicOp;
    features.features.multiDrawIndirect          = deviceFeatures.features.multiDrawIndirect;
    features.features.drawIndirectFirstInstance  = deviceFeatures.features.drawIndirectFirstInstance;
    features.features.depthClamp                 = deviceFeatures.features.depthClamp;
    features.features.depthBiasClamp             = deviceFeatures.features.depthBiasClamp;
    features.features.fillModeNonSolid           = deviceFeatures.features.fillModeNonSolid;
    features.features.depthBounds                = deviceFeatures.features.depthBounds;
    features.features.wideLines                  = deviceFeatures.features.wideLines;
    features.features.largePoints                = deviceFeatures.features.largePoints;
    features.features.alphaToOne                 = deviceFeatures.features.alphaToOne;
    features.features.multiViewport              = deviceFeatures.features.multiViewport;
    features.features.samplerAnisotropy          = deviceFeatures.features.samplerAnisotropy;
    features.features.textureCompressionETC2     = deviceFeatures.features.textureCompressionETC2;
    features.features.textureCompressionASTC_LDR = deviceFeatures.features.textureCompressionASTC_LDR;
    features.features.textureCompressionBC       = deviceFeatures.features.textureCompressionBC;
    features.features.shaderInt16                = deviceFeatures.features.shaderInt16;

    if (features.features.textureCompressionETC2) {
        _compressions |= TextureCompressionFlags::ETC2;
    }
    if (features.features.textureCompressionASTC_LDR) {
        _compressions |= TextureCompressionFlags::ASTC_LDR;
    }
    if (features.features.textureCompressionBC) {
        _compressions |= TextureCompressionFlags::BC;
    }

    if (_meshShaderSupported) {
        meshShaderFeatures.pNext                                  = features11.pNext;
        meshShaderFeatures.multiviewMeshShader                    = VK_FALSE;
        meshShaderFeatures.primitiveFragmentShadingRateMeshShader = VK_FALSE;
        meshShaderFeatures.meshShaderQueries                      = VK_FALSE;
        features11.pNext                                          = &meshShaderFeatures;
    }

    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.pNext                   = &features;
    createInfo.queueCreateInfoCount    = (uint32_t) queueCreateInfoCount;
    createInfo.pQueueCreateInfos       = queueCreateInfos;
    createInfo.enabledLayerCount       = (uint32_t) layers.size();
    createInfo.ppEnabledLayerNames     = layers.data();
    createInfo.enabledExtensionCount   = (uint32_t) extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

    check(_vkTable.vkCreateDevice(_physicalDevice, &createInfo, getAllocCalls(), &_device));
    _vkTable.init(vk::Device(_device));

    _vkTable.vkGetDeviceQueue(_device, graphic.index, graphic.queue, &_queueGraphic);
    _vkTable.vkGetDeviceQueue(_device, compute.index, compute.queue, &_queueCompute);
    _vkTable.vkGetDeviceQueue(_device, present.index, present.queue, &_queuePresent);
    _vkTable.vkGetDeviceQueue(_device, transfer.index, transfer.queue, &_queueTransfer);

    _commandBufferPool.create(*this, threadCount, 10, QueueType::Graphics);
    _frameCommandBuffer = getPrimaryCommandBuffer(false);
}

void Device::createProfiler(uint16_t gpuTimeQueriesPerFrame) {
    if (_profilerSupported) {
        VkProfiler* profiler;
        Object::create<VkProfiler>(profiler, *this, gpuTimeQueriesPerFrame, kMaxFrames);
        _profiler = profiler;
        profiler->destroy();

        VkQueryPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
        createInfo.queryType          = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount         = gpuTimeQueriesPerFrame * 2 * kMaxFrames;
        createInfo.pipelineStatistics = 0;
        check(_vkTable.vkCreateQueryPool(_device, &createInfo, getAllocCalls(), &_queryPool));

        _gpuFrequency = _deviceProperties.limits.timestampPeriod / (1'000'000.0);
    }
}

void Device::createDescriptorPools() {
    VkDescriptorPoolSize poolSizes[] = {
        // { VK_DESCRIPTOR_TYPE_SAMPLER,                kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kGlobalPoolElements * 2 },
        // { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          kGlobalPoolElements     },
        //{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   kGlobalPoolElements     },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   kGlobalPoolElements     },
        // { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         kGlobalPoolElements     },
        // { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       kGlobalPoolElements     }
    };

    VkDescriptorPoolCreateFlags flags = {};
    if (_bindlessSupported) {
        flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    }

    VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.flags         = flags;
    poolInfo.maxSets       = kDescriptorSetsPoolSize;
    poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
    poolInfo.pPoolSizes    = poolSizes;

    for (auto& pool : _descriptorPools) {
        check(_vkTable.vkCreateDescriptorPool(_device, &poolInfo, getAllocCalls(), &pool));
    }

    poolInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    check(_vkTable.vkCreateDescriptorPool(_device, &poolInfo, getAllocCalls(), &_globalDescriptorPool));
}

void Device::createVmaAllocator() {
    VmaVulkanFunctions functions{};
    functions.vkGetInstanceProcAddr = _vkTable.vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr   = _vkTable.vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo createInfo{};
    createInfo.flags                = _memoryBudgetSupported ? VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT : 0;
    createInfo.vulkanApiVersion     = VK_API_VERSION_1_2;
    createInfo.physicalDevice       = _physicalDevice;
    createInfo.device               = _device;
    createInfo.instance             = _instance;
    createInfo.pAllocationCallbacks = getAllocCalls();
    createInfo.pVulkanFunctions     = &functions;

    check(vmaCreateAllocator(&createInfo, &_vmaAllocator));
}

void Device::createDynamicBuffers() {
    const auto maxUBO = std::min((int) _deviceProperties.limits.maxUniformBufferRange, 65536);
    _dynamicUBOSize   = align(maxUBO, _alignment);

    BufferCreation bcUBO;
    bcUBO
        .set(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_INDEX_BIT | GERIUM_BUFFER_USAGE_UNIFORM_BIT |
                 GERIUM_BUFFER_USAGE_STORAGE_BIT | GERIUM_BUFFER_USAGE_INDIRECT_BIT,
             ResourceUsageType::Staging,
             _dynamicUBOSize * kMaxFrames)
        .setPersistent(true)
        .setName("Dynamic_Persistent_UBO");
    _dynamicUBO       = createBuffer(bcUBO);
    _dynamicUBOMapped = (uint8_t*) _buffers.access(_dynamicUBO)->mappedData;

    _dynamicSSBOSize = align(1024 * 1024 * 256, _alignment);

    BufferCreation bcSSBO;
    bcSSBO
        .set(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_INDEX_BIT | GERIUM_BUFFER_USAGE_STORAGE_BIT |
                 GERIUM_BUFFER_USAGE_INDIRECT_BIT,
             ResourceUsageType::Staging,
             _dynamicSSBOSize * kMaxFrames)
        .setPersistent(true)
        .setName("Dynamic_Persistent_SSBO");
    _dynamicSSBO       = createBuffer(bcSSBO);
    _dynamicSSBOMapped = (uint8_t*) _buffers.access(_dynamicSSBO)->mappedData;
}

void Device::createDefaultSampler() {
    SamplerCreation sc{};
    sc.setAddressModeUvw(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                         VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
                         VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
        .setMinMagMip(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR)
        .setName("Sampler Default");
    _defaultSampler = createSampler(sc);
}

void Device::createDefaultTexture() {
    gerium_uint32_t black = 0;
    TextureCreation tc{};
    tc.setSize(1, 1, 1)
        .setFlags(1, 1, false, false)
        .setFormat(GERIUM_FORMAT_R8G8B8A8_UNORM, GERIUM_TEXTURE_TYPE_2D)
        .setData(&black)
        .setName("default_texture")
        .build();
    _defaultTexture = createTexture(tc);
    tc.setFormat(GERIUM_FORMAT_R8G8B8A8_UNORM, GERIUM_TEXTURE_TYPE_3D).setName("default_texture_3d").build();
    _defaultTexture3D = createTexture(tc);
}

void Device::createSynchronizations() {
    VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < kMaxFrames; ++i) {
        check(_vkTable.vkCreateSemaphore(_device, &semaphoreInfo, getAllocCalls(), &_imageAvailableSemaphores[i]));
        check(_vkTable.vkCreateSemaphore(_device, &semaphoreInfo, getAllocCalls(), &_renderFinishedSemaphores[i]));
        check(_vkTable.vkCreateFence(_device, &fenceInfo, getAllocCalls(), &_inFlightFences[i]));
    }
}

void Device::createSwapchain(Application* application) {
    const auto swapchain   = getSwapchain();
    const auto format      = selectSwapchainFormat(swapchain.formats);
    const auto presentMode = selectSwapchainPresentMode(swapchain.presentModes);
    const auto extent      = selectSwapchainExtent(swapchain.capabilities, application);

    const auto imageCount =
        std::clamp(3U,
                   swapchain.capabilities.minImageCount,
                   std::max(swapchain.capabilities.maxImageCount, swapchain.capabilities.minImageCount));

    std::set<uint32_t> families = { _queueFamilies.graphic.value().index, _queueFamilies.present.value().index };
    if (families.size() == 1) {
        families.clear();
    }
    uint32_t indices[] = { _queueFamilies.graphic.value().index, _queueFamilies.present.value().index };

    auto sharingMode = families.empty() ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;

    VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    createInfo.surface               = _surface;
    createInfo.minImageCount         = imageCount;
    createInfo.imageFormat           = format.format;
    createInfo.imageColorSpace       = format.colorSpace;
    createInfo.imageExtent           = extent;
    createInfo.imageArrayLayers      = 1;
    createInfo.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    createInfo.imageSharingMode      = sharingMode;
    createInfo.queueFamilyIndexCount = (uint32_t) families.size();
    createInfo.pQueueFamilyIndices   = families.empty() ? nullptr : indices;
    createInfo.preTransform          = swapchain.capabilities.currentTransform;
    createInfo.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode           = presentMode;
    createInfo.clipped               = VK_TRUE;
    createInfo.oldSwapchain          = _swapchain;

    check(_vkTable.vkCreateSwapchainKHR(_device, &createInfo, getAllocCalls(), &_swapchain));

    _swapchainFormat = format;
    _swapchainExtent = extent;

    if (_swapchainRenderPass == Undefined) {
        RenderPassCreation rc{};
        rc.setName("SwapchainRenderPass");
        rc.output.color(_swapchainFormat.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, RenderPassOp::DontCare);
        rc.output.depth(VK_FORMAT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        rc.output.setDepthStencilOperations(RenderPassOp::Clear, RenderPassOp::Clear);
        _swapchainRenderPass = createRenderPass(rc);
    }

    uint32_t swapchainImages = 0;
    check(_vkTable.vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImages, nullptr));

    std::vector<VkImage> images;
    images.resize(swapchainImages);
    check(_vkTable.vkGetSwapchainImagesKHR(_device, _swapchain, &swapchainImages, images.data()));

    for (auto& handle : _swapchainFramebuffers) {
        destroyFramebuffer(handle);
    }
    _swapchainFramebuffers.resize(swapchainImages);

    auto commandBuffer = getPrimaryCommandBuffer(false);

    for (uint32_t i = 0; i < swapchainImages; ++i) {
        auto [colorHandle, color] = _textures.obtain_and_access();
        _swapchainImages.insert(colorHandle);

        color->vkImage       = images[i];
        color->layers        = 1;
        color->parentTexture = Undefined;
        color->sampler       = Undefined;
        color->loadedMips    = 1;

        VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        viewInfo.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                      = createInfo.imageFormat;
        viewInfo.image                       = images[i];
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.layerCount = 1;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.components.r                = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g                = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b                = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a                = VK_COMPONENT_SWIZZLE_A;
        check(_vkTable.vkCreateImageView(_device, &viewInfo, getAllocCalls(), &color->vkImageView));

        // TextureCreation depthCreation{};
        // depthCreation.setName("SwapchainDepthStencilTexture");
        // depthCreation.setSize(_swapchainExtent.width, _swapchainExtent.height, 1);
        // depthCreation.setFlags(1, false, false);
        // depthCreation.setFormat(GERIUM_FORMAT_D32_SFLOAT, GERIUM_TEXTURE_TYPE_2D);

        // auto depthHandle = createTexture(depthCreation);
        // auto depth       = _textures.access(depthHandle);

        auto name = "SwapchainFramebuffer"s + std::to_string(i);

        FramebufferCreation fc;
        fc.setName(name.c_str());
        fc.setScaling(1.0f, 1.0f, 0);
        fc.addRenderTexture(colorHandle);
        // fc.setDepthStencilTexture(depthHandle);
        fc.width      = _swapchainExtent.width;
        fc.height     = _swapchainExtent.height;
        fc.renderPass = _swapchainRenderPass;

        _swapchainFramebuffers[i] = createFramebuffer(fc);

        _textures.release(colorHandle);
        // _textures.release(depthHandle);

        commandBuffer->addImageBarrier(colorHandle, ResourceState::Present, 0, 1);
    }

    commandBuffer->submit(QueueType::Graphics);
}

void Device::createImGui(Application* application) {
    auto renderPass = _renderPasses.access(_swapchainRenderPass)->vkRenderPass;

    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 }
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets                    = 1000;
    poolInfo.poolSizeCount              = std::size(poolSizes);
    poolInfo.pPoolSizes                 = poolSizes;
    check(_vkTable.vkCreateDescriptorPool(_device, &poolInfo, getAllocCalls(), &_imguiPool));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    application->initImGui();

    ImGui_ImplVulkan_LoadFunctions(imguiLoaderFunc, this);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance            = _instance;
    initInfo.PhysicalDevice      = _physicalDevice;
    initInfo.Device              = _device;
    initInfo.QueueFamily         = _queueFamilies.graphic.value().index;
    initInfo.Queue               = _queueGraphic;
    initInfo.DescriptorPool      = _imguiPool;
    initInfo.RenderPass          = renderPass;
    initInfo.MinImageCount       = (uint32_t) _swapchainFramebuffers.size();
    initInfo.ImageCount          = (uint32_t) _swapchainFramebuffers.size();
    initInfo.MSAASamples         = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Subpass             = 0;
    initInfo.UseDynamicRendering = false;
    initInfo.Allocator           = getAllocCalls();
    initInfo.CheckVkResultFn     = check;
    ImGui_ImplVulkan_Init(&initInfo);

    ImGuiIO& io            = ImGui::GetIO();
    io.BackendRendererName = "gerium-ui";
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.DisplaySize = ImVec2{ float(_swapchainExtent.width), float(_swapchainExtent.height) };

    auto fs   = cmrc::gerium::resources::get_filesystem();
    auto font = fs.open("resources/OpenSans-Regular.ttf");

    // Add API to obtain DPI and other metrics
    // TODO: add calc density
    auto density  = 1.0f;
    auto fontSize = 24.0f;
    auto fontD    = 1.5f;
#ifdef GERIUM_PLATFORM_ANDROID
    density  = 1.5f;
    fontSize = 12.0f;
    fontD    = 1.4f;
#elif defined(GERIUM_PLATFORM_MAC_OS)
    density  = 1.0f;
    fontSize = 16.0f;
    fontD    = 2.0f;
#endif

    auto dataFont = IM_ALLOC(font.size());
    memcpy(dataFont, (void*) font.begin(), font.size());
    ImFontConfig config{};
    config.RasterizerDensity = fontD;
    io.Fonts->AddFontFromMemoryTTF(dataFont, (int) font.size(), fontSize * density, &config);
    ImGui::GetStyle().ScaleAllSizes(density);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void Device::createFidelityFX() {
    _ffxDeviceContext.vkInstance         = _instance;
    _ffxDeviceContext.vkDevice           = _device;
    _ffxDeviceContext.vkPhysicalDevice   = _physicalDevice;
    _ffxDeviceContext.vkInstanceProcAddr = _vkTable.vkGetInstanceProcAddr;
    _ffxDeviceContext.vkDeviceProcAddr   = _vkTable.vkGetDeviceProcAddr;
}

void Device::resizeSwapchain() {
    _vkTable.vkDeviceWaitIdle(_device);

    auto oldSwapchain = _swapchain;
    createSwapchain(_application);

    if (oldSwapchain) {
        _vkTable.vkDestroySwapchainKHR(_device, oldSwapchain, getAllocCalls());
    }

    for (auto descriptorSet : _descriptorSets) {
        if (descriptorSet->global) {
            descriptorSet->changed = 1;
        }
    }
}

void Device::printValidationLayers() {
    if (_enableValidations) {
        uint32_t count = 0;
        check(_vkTable.vkEnumerateInstanceLayerProperties(&count, nullptr));
        if (count == 0) {
            return;
        }

        std::vector<VkLayerProperties> layers;
        layers.resize(count);
        check(_vkTable.vkEnumerateInstanceLayerProperties(&count, layers.data()));

        _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, "Supported validation layers:");
        for (const auto& layer : layers) {
            _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, [&layer](auto& stream) {
                stream << "    "sv << layer.layerName << "(ver "sv << VK_API_VERSION_VARIANT(layer.specVersion) << '.'
                       << VK_API_VERSION_MAJOR(layer.specVersion) << '.' << VK_API_VERSION_MINOR(layer.specVersion)
                       << '.' << VK_API_VERSION_PATCH(layer.specVersion) << ", impl ver "sv
                       << layer.implementationVersion << ") -- "sv << layer.description;
            });
        }
        _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, "");
    }
}

void Device::printExtensions() {
    if (_enableValidations) {
        uint32_t count = 0;
        check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        if (count == 0) {
            return;
        }

        std::vector<VkExtensionProperties> extensions;
        extensions.resize(count);
        check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));

        _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, "Supported extensions:");
        for (const auto& extension : extensions) {
            _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, [&extension](auto& stream) {
                stream << "    "sv << extension.extensionName << " (ver "sv << extension.specVersion << ')';
            });
        }
        _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, "");
    }
}

void Device::printPhysicalDevices() {
    if (_enableValidations) {
        uint32_t count = 0;
        check(_vkTable.vkEnumeratePhysicalDevices(_instance, &count, nullptr));

        std::vector<VkPhysicalDevice> devices;
        devices.resize(count);
        check(_vkTable.vkEnumeratePhysicalDevices(_instance, &count, devices.data()));

        _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, "GPU Devices:");
        VkPhysicalDeviceProperties props;
        for (const auto& device : devices) {
            _vkTable.vkGetPhysicalDeviceProperties(device, &props);
            const char* deviceType = "unknown";
            switch (props.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                    deviceType = "integrated GPU";
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                    deviceType = "discrete GPU";
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                    deviceType = "virtual GPU";
                    break;
                case VK_PHYSICAL_DEVICE_TYPE_CPU:
                    deviceType = "CPU";
                    break;
                default:
                    assert(!"unreachable code");
                    break;
            }

            _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, [&props, deviceType](auto& stream) {
                stream << "    "sv << props.deviceName << " (type '"sv << deviceType << "', id '"sv << props.deviceID
                       << "', vendor id '"sv << props.vendorID << "', api ver '"sv
                       << VK_API_VERSION_VARIANT(props.apiVersion) << '.' << VK_API_VERSION_MAJOR(props.apiVersion)
                       << '.' << VK_API_VERSION_MINOR(props.apiVersion) << '.' << VK_API_VERSION_PATCH(props.apiVersion)
                       << "', driver ver '"sv << VK_API_VERSION_VARIANT(props.driverVersion) << '.'
                       << VK_API_VERSION_MAJOR(props.driverVersion) << '.' << VK_API_VERSION_MINOR(props.driverVersion)
                       << '.' << VK_API_VERSION_PATCH(props.driverVersion) << '\'';
            });
        }
        _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, "");
    }
}

std::tuple<uint32_t, bool> Device::fillWriteDescriptorSets(const DescriptorSetLayout& descriptorSetLayout,
                                                           const DescriptorSet& descriptorSet,
                                                           VkWriteDescriptorSet* descriptorWrite,
                                                           VkDescriptorBufferInfo* bufferInfo,
                                                           VkDescriptorImageInfo* imageInfo) {
    uint32_t i          = 0;
    bool updateRequired = false;

    auto defaultSampler = _samplers.access(_defaultSampler)->vkSampler;

    for (const auto& [_, item] : descriptorSet.bindings) {
        auto resource = item.handle;

        auto it = std::find_if(descriptorSetLayout.data.bindings.cbegin(),
                               descriptorSetLayout.data.bindings.cend(),
                               [b = item.binding](const auto& binding) {
            return binding.binding == b;
        });

        if (it == descriptorSetLayout.data.bindings.cend()) {
            continue;
        }

        const auto& binding = *it;

        descriptorWrite[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[i].dstSet          = descriptorSet.vkDescriptorSet;
        descriptorWrite[i].dstBinding      = item.binding;
        descriptorWrite[i].dstArrayElement = item.element;
        descriptorWrite[i].descriptorCount = 1;

        switch (binding.descriptorType) {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                auto texture = _textures.access(
                    resource != Undefined ? resource : getDefaultTexture(descriptorSetLayout, binding.binding));

                imageInfo[i].sampler =
                    texture->sampler != Undefined ? _samplers.access(texture->sampler)->vkSampler : defaultSampler;

                if (texture->loadedMips == 0) {
                    texture        = _textures.access(getDefaultTexture(descriptorSetLayout, binding.binding));
                    updateRequired = true;
                }

                imageInfo[i].imageLayout = hasDepthOrStencil(texture->vkFormat)
                                               ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                               : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                imageInfo[i].imageView = texture->vkImageView;

                descriptorWrite[i].pImageInfo = &imageInfo[i];
                break;
            }
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
                if (resource == Undefined) {
                    continue;
                }
                descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

                auto texture = _textures.access(resource);

                imageInfo[i].sampler     = VK_NULL_HANDLE;
                imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                imageInfo[i].imageView   = texture->vkImageView;

                descriptorWrite[i].pImageInfo = &imageInfo[i];
                break;
            }
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
                if (resource == Undefined) {
                    continue;
                }
                auto buffer = _buffers.access(resource);

                descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

                if (buffer->parent != Undefined) {
                    bufferInfo[i].buffer = _buffers.access(buffer->parent)->vkBuffer;
                } else {
                    bufferInfo[i].buffer = buffer->vkBuffer;
                }

                bufferInfo[i].offset = 0;
                bufferInfo[i].range  = buffer->size;

                descriptorWrite[i].pBufferInfo = &bufferInfo[i];
                break;
            }
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                if (resource == Undefined) {
                    continue;
                }
                auto buffer = _buffers.access(resource);

                descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;

                if (buffer->parent != Undefined) {
                    bufferInfo[i].buffer = _buffers.access(buffer->parent)->vkBuffer;
                } else {
                    bufferInfo[i].buffer = buffer->vkBuffer;
                }

                bufferInfo[i].offset = 0;
                bufferInfo[i].range  = buffer->size;

                descriptorWrite[i].pBufferInfo = &bufferInfo[i];
                break;
            }
            default: {
                assert(!"Resource not supported in descriptor set creation!");
                break;
            }
        }
        ++i;
    }

    return { i, updateRequired };
}

std::vector<uint32_t> Device::compile(const char* code,
                                      size_t size,
                                      gerium_shader_languge_t lang,
                                      VkShaderStageFlagBits stage,
                                      const char* name,
                                      gerium_uint32_t numMacros,
                                      const gerium_macro_definition_t* macros) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    shaderc_source_language sourceLang;
    shaderc_shader_kind kind;

    switch (lang) {
        case GERIUM_SHADER_LANGUAGE_GLSL:
            sourceLang = shaderc_source_language_glsl;
            break;
        case GERIUM_SHADER_LANGUAGE_HLSL:
            sourceLang = shaderc_source_language_hlsl;
            break;
        default:
            throw std::runtime_error("Not supported language");
    }

    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            options.AddMacroDefinition("VERTEX_SHADER"s, "1"s);
            kind = shaderc_glsl_vertex_shader;
            break;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            options.AddMacroDefinition("FRAGMENT_SHADER"s, "1"s);
            kind = shaderc_glsl_fragment_shader;
            break;
        case VK_SHADER_STAGE_GEOMETRY_BIT:
            options.AddMacroDefinition("GEOMETRY_SHADER"s, "1"s);
            kind = shaderc_glsl_geometry_shader;
            break;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            options.AddMacroDefinition("COMPUTE_SHADER"s, "1"s);
            kind = shaderc_glsl_compute_shader;
            break;
        case VK_SHADER_STAGE_TASK_BIT_EXT:
            options.AddMacroDefinition("TASK_SHADER"s, "1"s);
            kind = shaderc_glsl_task_shader;
            break;
        case VK_SHADER_STAGE_MESH_BIT_EXT:
            options.AddMacroDefinition("MESH_SHADER"s, "1"s);
            kind = shaderc_glsl_mesh_shader;
            break;
        default:
            throw std::runtime_error("Not supported shader type");
    }

    if (_bindlessSupported) {
        options.AddMacroDefinition("BINDLESS_SUPPORTED"s, "1"s);
    }

    if (_meshShaderSupported) {
        options.AddMacroDefinition("MESH_SHADER_SUPPORTED"s, "1"s);
    }

    if (_samplerFilterMinmaxSupported) {
        options.AddMacroDefinition("SAMPLER_FILTER_MINMAX_SUPPORTED"s, "1"s);
    }

    if (_fidelityFXSupported) {
        options.AddMacroDefinition("FIDELITY_FX_SUPPORTED"s, "1"s);
    }

    if (_8BitStorageSupported) {
        options.AddMacroDefinition("SHADER_8BIT_STORAGE_SUPPORTED"s, "1"s);
    }

    if (_16BitStorageSupported) {
        options.AddMacroDefinition("SHADER_16BIT_STORAGE_SUPPORTED"s, "1"s);
    }

    if (!_enableValidations) {
        options.AddMacroDefinition("NDEBUG"s, "1"s);
    }

    for (gerium_uint32_t i = 0; i < numMacros; ++i) {
        const auto& macro = macros[i];
        options.AddMacroDefinition(macro.name, macro.value);
    }

    class Includer : public shaderc::CompileOptions::IncluderInterface {
    public:
        explicit Includer(const std::filesystem::path fullpath) : path(fullpath) {
        }

        shaderc_include_result* GetInclude(const char* requested_source,
                                           shaderc_include_type type,
                                           const char* requesting_source,
                                           size_t include_depth) override {
            auto result = new shaderc_include_result{};

            result->source_name    = "";
            result->content        = "Include file not found";
            result->content_length = 22;

            if (type == shaderc_include_type_relative) {
                auto includePath = std::filesystem::path(requested_source);
                if (!includePath.is_absolute()) {
                    auto requestPath = std::filesystem::path(requesting_source);
                    if (requestPath.is_absolute()) {
                        includePath = requestPath.parent_path() / requested_source;
                    } else {
                        includePath = path / requested_source;
                    }
                }
                auto includePathStr = includePath.make_preferred().string();
                if (File::existsFile(includePathStr.c_str())) {
                    auto file       = File::open(includePathStr.c_str(), true);
                    const auto size = file->getSize();

                    includePaths.emplace_back(new char[includePathStr.length() + 1]{});
                    files.emplace_back(new char[size + 1]{});

                    memcpy((void*) includePaths.back().get(), includePathStr.c_str(), includePathStr.length());
                    file->read((gerium_data_t) files.back().get(), (gerium_uint32_t) size);

                    result->source_name        = includePaths.back().get();
                    result->source_name_length = includePathStr.length();
                    result->content            = files.back().get();
                    result->content_length     = size;
                }
            }

            return result;
        }

        void ReleaseInclude(shaderc_include_result* data) override {
            delete data;
        }

        std::filesystem::path path;
        std::vector<std::unique_ptr<const char[]>> includePaths;
        std::vector<std::unique_ptr<const char[]>> files;
    };

    options.SetSourceLanguage(sourceLang);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
    options.SetTargetSpirv(shaderc_spirv_version_1_4);
    options.SetWarningsAsErrors();
    options.SetPreserveBindings(true);
    options.SetAutoMapLocations(true);
    options.SetAutoBindUniforms(true);
    options.SetAutoSampledTextures(true);
    options.SetIncluder(std::make_unique<Includer>(File::getAppDir() / std::filesystem::path(name).parent_path()));

    auto result = compiler.CompileGlslToSpv(code, size, kind, name, "main", options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::stringstream ss(result.GetErrorMessage());
        std::string message;
        while (std::getline(ss, message, '\n')) {
            _logger->print(GERIUM_LOGGER_LEVEL_ERROR, message.c_str());
        }
        error(GERIUM_RESULT_ERROR_COMPILE_SHADER);
    }

    return { result.cbegin(), result.cend() };
}

VkRenderPass Device::vkCreateRenderPass(const RenderPassOutput& output, const char* name) {
    VkAttachmentDescription attachmets[kMaxImageOutputs + 1]{};
    VkAttachmentReference colorAttachmentsRef[kMaxImageOutputs]{};
    VkAttachmentReference depthAttachmentRef{};

    VkAttachmentLoadOp depthOp;
    VkAttachmentLoadOp stencilOp;
    VkImageLayout depthInitial;

    switch (output.depthOperation) {
        case RenderPassOp::DontCare:
            depthOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthInitial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        case RenderPassOp::Load:
            depthOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case RenderPassOp::Clear:
            depthOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthInitial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        default:
            assert(!"unreachable code");
            break;
    }

    switch (output.stencilOperation) {
        case RenderPassOp::DontCare:
            stencilOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        case RenderPassOp::Load:
            stencilOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case RenderPassOp::Clear:
            stencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        default:
            assert(!"unreachable code");
            break;
    }

    auto isSwapchain = false;

    uint32_t attachmentCount = 0;
    for (; attachmentCount < output.numColorFormats; ++attachmentCount) {
        VkAttachmentLoadOp colorOp;
        VkImageLayout colorInitial;
        switch (output.colorOperations[attachmentCount]) {
            case RenderPassOp::DontCare:
                colorOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorInitial = VK_IMAGE_LAYOUT_UNDEFINED;
                break;
            case RenderPassOp::Load:
                colorOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            case RenderPassOp::Clear:
                colorOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
                colorInitial = VK_IMAGE_LAYOUT_UNDEFINED;
                break;
            default:
                assert(!"unreachable code");
                break;
        }

        auto& colorAttachment          = attachmets[attachmentCount];
        colorAttachment.format         = output.colorFormats[attachmentCount];
        colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp         = colorOp;
        colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp  = stencilOp;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout  = colorInitial;
        colorAttachment.finalLayout    = output.colorFinalLayouts[attachmentCount];

        VkAttachmentReference& colorAttachmentRef = colorAttachmentsRef[attachmentCount];
        colorAttachmentRef.attachment             = attachmentCount;
        colorAttachmentRef.layout                 = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (colorAttachment.finalLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
            isSwapchain = true;
        }
    }

    if (output.depthStencilFormat != VK_FORMAT_UNDEFINED) {
        auto& depthAttachment         = attachmets[attachmentCount];
        depthAttachment.format        = output.depthStencilFormat;
        depthAttachment.samples       = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp        = depthOp;
        depthAttachment.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = stencilOp;
        depthAttachment.stencilStoreOp =
            hasStencil(output.depthStencilFormat) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = depthInitial;
        depthAttachment.finalLayout   = output.depthStencilFinalLayout;

        depthAttachmentRef.attachment = attachmentCount++;
        depthAttachmentRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.inputAttachmentCount    = 0;
    subpass.pInputAttachments       = nullptr;
    subpass.colorAttachmentCount    = output.numColorFormats;
    subpass.pColorAttachments       = colorAttachmentsRef;
    subpass.pResolveAttachments     = nullptr;
    subpass.pDepthStencilAttachment = output.depthStencilFormat != VK_FORMAT_UNDEFINED ? &depthAttachmentRef : nullptr;
    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments    = nullptr;

    VkSubpassDependency dependencies[kMaxImageOutputs + 1];
    uint32_t dependencyCount = 0;

    if (isSwapchain) {
        auto& dependency           = dependencies[dependencyCount++];
        dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass      = 0;
        dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask   = VK_ACCESS_NONE;
        dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;

        auto& dependency2           = dependencies[dependencyCount++];
        dependency2.srcSubpass      = 0;
        dependency2.dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependency2.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency2.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency2.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency2.dstAccessMask   = VK_ACCESS_NONE;
        dependency2.dependencyFlags = 0;
    }

    VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments    = attachmets;
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = &subpass;
    createInfo.dependencyCount = dependencyCount;
    createInfo.pDependencies   = dependencies;

    VkRenderPass renderPass;
    check(_vkTable.vkCreateRenderPass(_device, &createInfo, getAllocCalls(), &renderPass));

    setObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t) renderPass, name);

    return renderPass;
}

void Device::vkCreateImageView(const TextureViewCreation& creation, TextureHandle handle) {
    auto texture = _textures.access(handle);

    VkImageAspectFlags aspectMask{};
    if (hasDepthOrStencil(texture->vkFormat)) {
        aspectMask |= hasDepth(texture->vkFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
        aspectMask |= hasStencil(texture->vkFormat) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
    } else {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image      = texture->vkImage;
    viewInfo.viewType   = toVkImageViewType(creation.type, creation.arrayLayerCount > 1);
    viewInfo.format     = texture->vkFormat;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY };

    viewInfo.subresourceRange.aspectMask     = aspectMask;
    viewInfo.subresourceRange.baseMipLevel   = creation.mipBaseLevel;
    viewInfo.subresourceRange.levelCount     = creation.mipLevelCount;
    viewInfo.subresourceRange.baseArrayLayer = creation.arrayBaseLayer;
    viewInfo.subresourceRange.layerCount     = creation.arrayLayerCount;

    check(_vkTable.vkCreateImageView(_device, &viewInfo, getAllocCalls(), &texture->vkImageView));
    setObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t) texture->vkImageView, creation.name);
}

void Device::deleteResources(bool forceDelete) {
    while (!_deletionQueue.empty()) {
        const auto& resource = _deletionQueue.front();
        if (resource.frame != _currentFrame && !forceDelete) {
            break;
        }
        switch (resource.type) {
            case ResourceType::Buffer:
                if (_buffers.references(BufferHandle{ resource.handle }) == 1) {
                    auto buffer = _buffers.access(resource.handle);
                    if (buffer->parent == Undefined) {
                        vmaDestroyBuffer(_vmaAllocator, buffer->vkBuffer, buffer->vmaAllocation);
                    }
                }
                _buffers.release(resource.handle);
                break;
            case ResourceType::Texture:
                if (_textures.references(TextureHandle{ resource.handle }) == 1) {
                    auto texture = _textures.access(resource.handle);
                    if (texture->sampler != Undefined) {
                        destroySampler(texture->sampler);
                    }
                    if (texture->vkImageView) {
                        _vkTable.vkDestroyImageView(_device, texture->vkImageView, getAllocCalls());
                    }
                    if (texture->vkImage && texture->vmaAllocation) {
                        vmaDestroyImage(_vmaAllocator, texture->vkImage, texture->vmaAllocation);
                    } else if (texture->vkImage && !_swapchainImages.contains(resource.handle) &&
                               texture->parentTexture == Undefined) {
                        _vkTable.vkDestroyImage(_device, texture->vkImage, getAllocCalls());
                    } else {
                        _swapchainImages.erase(TextureHandle{ resource.handle });
                    }
                    if (texture->parentTexture != Undefined) {
                        destroyTexture(texture->parentTexture);
                    }
                }
                _textures.release(resource.handle);
                break;
            case ResourceType::Sampler:
                if (_samplers.references(SamplerHandle{ resource.handle }) == 1) {
                    auto sampler = _samplers.access(resource.handle);
                    _samplerCache.erase(calcSamplerHash(
                        SamplerCreation()
                            .setMinMagMip(sampler->minFilter, sampler->magFilter, sampler->mipFilter)
                            .setAddressModeUvw(sampler->addressModeU, sampler->addressModeV, sampler->addressModeW)));
                    _vkTable.vkDestroySampler(_device, sampler->vkSampler, getAllocCalls());
                }
                _samplers.release(resource.handle);
                break;
            case ResourceType::RenderPass:
                if (_renderPasses.references(RenderPassHandle{ resource.handle }) == 1) {
                    auto renderPass = _renderPasses.access(resource.handle);
                    _renderPassCache.erase(hash(renderPass->output));
                    _vkTable.vkDestroyRenderPass(_device, renderPass->vkRenderPass, getAllocCalls());
                }
                _renderPasses.release(resource.handle);
                break;
            case ResourceType::Framebuffer:
                if (_framebuffers.references(FramebufferHandle{ resource.handle }) == 1) {
                    auto framebuffer = _framebuffers.access(resource.handle);
                    for (gerium_uint32_t i = 0; i < framebuffer->numColorAttachments; ++i) {
                        destroyTexture(framebuffer->colorAttachments[i]);
                    }
                    if (framebuffer->depthStencilAttachment != Undefined) {
                        destroyTexture(framebuffer->depthStencilAttachment);
                    }
                    if (framebuffer->renderPass != Undefined) {
                        destroyRenderPass(framebuffer->renderPass);
                    }
                    _vkTable.vkDestroyFramebuffer(_device, framebuffer->vkFramebuffer, getAllocCalls());
                }
                _framebuffers.release(resource.handle);
                break;
            case ResourceType::Program:
                if (_programs.references(ProgramHandle{ resource.handle }) == 1) {
                    auto program = _programs.access(resource.handle);
                    for (uint32_t i = 0; i < program->activeShaders; ++i) {
                        if (program->shaderStageInfo[i].module) {
                            _vkTable.vkDestroyShaderModule(
                                _device, program->shaderStageInfo[i].module, getAllocCalls());
                        }
                    }
                }
                _programs.release(resource.handle);
                break;
            case ResourceType::DescriptorSet:
                if (_descriptorSets.references(DescriptorSetHandle{ resource.handle }) == 1) {
                    auto descriptorSet = _descriptorSets.access(resource.handle);
                    if (descriptorSet->global && descriptorSet->vkDescriptorSet) {
                        _vkTable.vkFreeDescriptorSets(
                            _device, _globalDescriptorPool, 1, &descriptorSet->vkDescriptorSet);
                    }
                }
                _descriptorSets.release(resource.handle);
                break;
            case ResourceType::DescriptorSetLayout:
                if (_descriptorSetLayouts.references(DescriptorSetLayoutHandle{ resource.handle }) == 1) {
                    auto layout = _descriptorSetLayouts.access(resource.handle);
                    _vkTable.vkDestroyDescriptorSetLayout(_device, layout->vkDescriptorSetLayout, getAllocCalls());
                }
                _descriptorSetLayouts.release(resource.handle);
                break;
            case ResourceType::Pipeline:
                if (_pipelines.references(PipelineHandle{ resource.handle }) == 1) {
                    auto pipeline = _pipelines.access(resource.handle);
                    _vkTable.vkDestroyPipelineLayout(_device, pipeline->vkPipelineLayout, getAllocCalls());
                    _vkTable.vkDestroyPipeline(_device, pipeline->vkPipeline, getAllocCalls());
                    for (uint32_t i = 0; i < pipeline->numActiveLayouts; ++i) {
                        destroyDescriptorSetLayout(pipeline->descriptorSetLayoutHandles[i]);
                    }
                    if (pipeline->renderPass != Undefined) {
                        destroyRenderPass(pipeline->renderPass);
                    }
                }
                _pipelines.release(resource.handle);
                break;
        }
        _deletionQueue.pop();
    }
}

void Device::setObjectName(VkObjectType type, uint64_t handle, gerium_utf8_t name) {
    if (_enableDebugNames && name) {
        VkDebugUtilsObjectNameInfoEXT info{ VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
        info.objectType   = type;
        info.objectHandle = handle;
        info.pObjectName  = name;
        check(_vkTable.vkSetDebugUtilsObjectNameEXT(_device, &info));
    }
}

int Device::getPhysicalDeviceScore(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties props;
    _vkTable.vkGetPhysicalDeviceProperties(device, &props);

    VkPhysicalDeviceFeatures features;
    _vkTable.vkGetPhysicalDeviceFeatures(device, &features);

    int score = 0;

    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1;
    }

    if (!getQueueFamilies(device).isComplete()) {
        return 0;
    }

    if (!features.imageCubeArray) {
        return 0;
    }

    try {
        selectDeviceExtensions(device, false);
    } catch (const Exception& exc) {
        if (exc.result() == GERIUM_RESULT_ERROR_FEATURE_NOT_SUPPORTED) {
            return 0;
        }
        throw;
    }

    uint32_t countFormats  = 0;
    uint32_t countPresents = 0;
    check(_vkTable.vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &countFormats, nullptr));
    check(_vkTable.vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &countPresents, nullptr));

    if (!countFormats || !countPresents) {
        return 0;
    }

    return score + 1;
}

Device::QueueFamilies Device::getQueueFamilies(VkPhysicalDevice device) {
    QueueFamilies result{};

    int index                 = 0;
    VkBool32 surfaceSupported = VK_FALSE;

    uint32_t count = 0;
    _vkTable.vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> families;
    families.resize(count);
    _vkTable.vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

    constexpr auto graphic  = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    constexpr auto transfer = VK_QUEUE_TRANSFER_BIT;

    for (const auto& family : families) {
        if (family.queueCount == 0) {
            continue;
        }

        if (!result.graphic.has_value() && (family.queueFlags & graphic) == graphic) {
            result.graphic = { index, 0, family.timestampValidBits };
            if (family.queueCount > 1) {
                result.compute = { index, 1, family.timestampValidBits };
            }
        }

        if (!result.compute.has_value() && (family.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            result.compute = { index, 0, family.timestampValidBits };
        }

        if (!result.transfer.has_value() && (family.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0 &&
            (family.queueFlags & transfer) == transfer) {
            result.transfer = { index, 0, family.timestampValidBits };
        }

        check(_vkTable.vkGetPhysicalDeviceSurfaceSupportKHR(device, index, _surface, &surfaceSupported));

        if (!result.present.has_value() && surfaceSupported) {
            result.present = { index, 0, family.timestampValidBits };
        }

        if (result.isComplete()) {
            break;
        }

        ++index;
    }

    if (!result.transfer.has_value()) {
        result.transfer          = result.graphic;
        result.transferIsGraphic = true;
    }

    return result;
}

Device::Swapchain Device::getSwapchain() {
    Swapchain result;
    check(_vkTable.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &result.capabilities));

    uint32_t countFormats  = 0;
    uint32_t countPresents = 0;
    check(_vkTable.vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &countFormats, nullptr));
    check(_vkTable.vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &countPresents, nullptr));

    result.formats.resize(countFormats);
    result.presentModes.resize(countPresents);
    check(
        _vkTable.vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &countFormats, result.formats.data()));
    check(_vkTable.vkGetPhysicalDeviceSurfacePresentModesKHR(
        _physicalDevice, _surface, &countPresents, result.presentModes.data()));

    return result;
}

void Device::frameCountersAdvance() noexcept {
    _previousFrame = _currentFrame;
    _currentFrame  = (_currentFrame + 1) % kMaxFrames;
    ++_absoluteFrame;
}

void Device::uploadTextureData(TextureHandle handle, gerium_cdata_t data) {
    auto texture = _textures.access(handle);
    BufferCreation bc{};
    bc.set(GERIUM_BUFFER_USAGE_VERTEX_BIT, ResourceUsageType::Dynamic, texture->size);

    auto stagingBuffer = createBuffer(bc);

    auto mapped = mapBuffer(stagingBuffer);
    memcpy((void*) mapped, (void*) data, texture->size);
    unmapBuffer(stagingBuffer);

    _frameCommandBuffer->copyBuffer(stagingBuffer, handle, 0);
    _frameCommandBuffer->generateMipmaps(handle);
    destroyBuffer(stagingBuffer);

    texture->loadedMips = texture->mipLevels;
}

TextureHandle Device::getDefaultTexture(const DescriptorSetLayout& descriptorSetLayout,
                                        uint32_t binding) const noexcept {
    return descriptorSetLayout.data.default3DTextures.contains(binding) ? _defaultTexture3D : _defaultTexture;
}

std::vector<const char*> Device::selectValidationLayers() {
    return checkValidationLayers({ "VK_LAYER_KHRONOS_validation", "MoltenVK" });
}

std::vector<const char*> Device::selectExtensions() {
    std::vector<std::pair<const char*, bool>> extensions = {
        { VK_KHR_SURFACE_EXTENSION_NAME, true }
    };

    for (const auto& extension : onGetInstanceExtensions()) {
        extensions.emplace_back(extension, true);
    }

    if (_enableValidations) {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false);
    }

    return checkExtensions(extensions);
}

std::vector<const char*> Device::selectDeviceExtensions(VkPhysicalDevice device, bool meshShader) {
    std::vector<std::pair<const char*, bool>> extensions = {
        { VK_KHR_SWAPCHAIN_EXTENSION_NAME,                 true  },
        { VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,             false },
        { VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME, false }  // need FidelityFX
    };

    if (meshShader) {
        extensions.emplace_back(VK_EXT_MESH_SHADER_EXTENSION_NAME, false);
    }

    for (const auto& extension : onGetDeviceExtensions()) {
        extensions.emplace_back(extension, true);
    }

    return checkDeviceExtensions(device, extensions);
}

VkPhysicalDevice Device::selectPhysicalDevice() {
    uint32_t count = 0;
    check(_vkTable.vkEnumeratePhysicalDevices(_instance, &count, nullptr));

    if (!count) {
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, "failed to find GPUs with Vulkan API support");
        error(GERIUM_RESULT_ERROR_DEVICE_SELECTION);
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(count);
    check(_vkTable.vkEnumeratePhysicalDevices(_instance, &count, physicalDevices.data()));

    std::multimap<int, VkPhysicalDevice> devices;

    for (const auto& device : physicalDevices) {
        const auto score = getPhysicalDeviceScore(device);
        devices.insert(std::make_pair(score, device));
    }

    if (devices.rbegin()->first > 0) {
        return devices.rbegin()->second;
    }

    _logger->print(GERIUM_LOGGER_LEVEL_ERROR, "failed to find a suitable GPU");
    error(GERIUM_RESULT_ERROR_DEVICE_SELECTION);

    assert(!"unreachable code");
    return VK_NULL_HANDLE;
}

VkSurfaceFormatKHR Device::selectSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM || format.format == VK_FORMAT_R8G8B8A8_UNORM) {
            if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }
    }
    return formats[0];
}

VkPresentModeKHR Device::selectSwapchainPresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
    for (const auto& presentMode : presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Device::selectSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, Application* application) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        gerium_uint16_t width, height;
        application->getSize(&width, &height);

        VkExtent2D extent = { (uint32_t) width, (uint32_t) height };

        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height =
            std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
    }
}

std::vector<const char*> Device::checkValidationLayers(const std::vector<const char*>& layers) {
    std::vector<const char*> results;
    if (_enableValidations) {
        uint32_t count = 0;
        check(_vkTable.vkEnumerateInstanceLayerProperties(&count, nullptr));

        std::vector<VkLayerProperties> props;
        if (count) {
            props.resize(count);
            check(_vkTable.vkEnumerateInstanceLayerProperties(&count, props.data()));
        }

        for (const auto& prop : props) {
            for (const auto& layer : layers) {
                if (strncmp(prop.layerName, layer, 255) == 0) {
                    results.push_back(layer);
                    break;
                }
            }
        }

        for (const auto& layer : layers) {
            _logger->print(GERIUM_LOGGER_LEVEL_DEBUG, [&results, layer](auto& stream) {
                const auto found = contains(results, layer);
                stream << "Layer "sv << layer << ' ' << (found ? "found"sv : "not found"sv);
            });
        }
    }
    return results;
}

std::vector<const char*> Device::checkExtensions(const std::vector<std::pair<const char*, bool>>& extensions) {
    std::vector<const char*> results;

    uint32_t count = 0;
    check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

    std::vector<VkExtensionProperties> props;
    if (count) {
        props.resize(count);
        check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data()));
    }

    for (const auto& prop : props) {
        for (const auto& [extension, _] : extensions) {
            if (strncmp(prop.extensionName, extension, 255) == 0) {
                results.push_back(extension);
                break;
            }
        }
    }

    for (const auto& [extension, required] : extensions) {
        const auto notFound = !contains(results, extension);
        if (notFound && required) {
            check(VK_ERROR_EXTENSION_NOT_PRESENT);
        }
    }

    return results;
}

std::vector<const char*> Device::checkDeviceExtensions(VkPhysicalDevice device,
                                                       const std::vector<std::pair<const char*, bool>>& extensions) {
    std::vector<const char*> results;

    uint32_t count = 0;
    check(_vkTable.vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));

    std::vector<VkExtensionProperties> avaiableExtensions;
    avaiableExtensions.resize(count);
    check(_vkTable.vkEnumerateDeviceExtensionProperties(device, nullptr, &count, avaiableExtensions.data()));

    for (const auto& [extension, required] : extensions) {
        bool found = false;
        for (const auto& avaiable : avaiableExtensions) {
            if (strncmp(avaiable.extensionName, extension, 255) == 0) {
                found = true;
                results.push_back(extension);
                break;
            }
        }

        if (!found && required) {
            check(VK_ERROR_EXTENSION_NOT_PRESENT);
        }
    }

    return results;
}

bool Device::checkPhysicalDeviceExtensions(VkPhysicalDevice device, const std::vector<const char*>& extensions) {
    uint32_t count = 0;
    check(_vkTable.vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr));

    std::vector<VkExtensionProperties> avaiableExtensions;
    avaiableExtensions.resize(count);
    check(_vkTable.vkEnumerateDeviceExtensionProperties(device, nullptr, &count, avaiableExtensions.data()));

    for (const auto& extension : extensions) {
        bool found = false;
        for (const auto& avaiable : avaiableExtensions) {
            if (strncmp(avaiable.extensionName, extension, 255) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

void Device::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                           VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData) {
    gerium_logger_level_t level = GERIUM_LOGGER_LEVEL_VERBOSE;
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        level = GERIUM_LOGGER_LEVEL_ERROR;
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        level = GERIUM_LOGGER_LEVEL_WARNING;
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        level = GERIUM_LOGGER_LEVEL_INFO;
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        level = GERIUM_LOGGER_LEVEL_VERBOSE;
    }

    _logger->print(level, [data = pCallbackData](auto& stream) {
        stream << " mess id name '"sv << (data->pMessageIdName ? data->pMessageIdName : "<none>")
               << "', mess id num '"sv << data->messageIdNumber << "', mess '"sv << data->pMessage << '\'';
    });
}

gerium_uint64_t Device::calcPipelineHash(const PipelineCreation& creation) noexcept {
    gerium_uint64_t seed = hash(GERIUM_VERSION);

    seed = hash(*creation.rasterization, seed);
    seed = hash(*creation.depthStencil, seed);
    seed = hash(*creation.colorBlend, seed);
    seed = hash(creation.blendState, seed);
    seed = hash(creation.vertexInput, seed);
    seed = hash(creation.renderPass, seed);

    for (gerium_uint32_t i = 0; i < creation.program.stagesCount; ++i) {
        const auto& stage = creation.program.stages[i];

        seed = hash(stage.data, stage.size, seed);

        for (gerium_uint32_t m = 0; m < stage.macro_count; ++m) {
            const auto& macro = stage.macros[m];

            if (stage.entry_point) {
                seed = hash(stage.entry_point, seed);
            }
            seed = hash(macro.name, seed);
            seed = hash(macro.value, seed);
        }
    }

    return seed;
}

gerium_uint64_t Device::calcSamplerHash(const SamplerCreation& creation) noexcept {
    gerium_uint64_t seed = hash(GERIUM_VERSION);

    seed = hash(creation.minFilter, seed);
    seed = hash(creation.magFilter, seed);
    seed = hash(creation.mipFilter, seed);
    seed = hash(creation.addressModeU, seed);
    seed = hash(creation.addressModeV, seed);
    seed = hash(creation.addressModeW, seed);
    seed = hash(creation.reductionMode, seed);

    return seed;
}

gerium_uint32_t Device::calcBindingKey(gerium_uint16_t binding, gerium_uint16_t element) noexcept {
    return (binding << 16) | element;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
Device::debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                    void* pUserData) {
    auto device = (Device*) pUserData;
    device->debugCallback(messageSeverity, messageTypes, pCallbackData);
    return VK_FALSE;
}

PFN_vkVoidFunction Device::imguiLoaderFunc(const char* functionName, void* userData) {
    auto device = (Device*) userData;
    return device->_vkTable.vkGetInstanceProcAddr(device->_instance, functionName);
}

VkInstanceCreateFlags Device::onGetCreateInfoFlags() const noexcept {
    return 0;
}

const void* Device::onGetNextCreateInfo(void* pNext) noexcept {
    return pNext;
}

std::vector<const char*> Device::onGetInstanceExtensions() const noexcept {
    return {};
}

std::vector<const char*> Device::onGetDeviceExtensions() const noexcept {
    return {};
}

bool Device::onNeedPostAcquireResize() const noexcept {
    return false;
}

} // namespace gerium::vulkan
