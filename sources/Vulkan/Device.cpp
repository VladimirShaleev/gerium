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

        for (uint32_t i = 0; i < MaxFrames; ++i) {
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

        if (_descriptorPool) {
            _vkTable.vkDestroyDescriptorPool(_device, _descriptorPool, getAllocCalls());
        }

        _profiler.reset();

        if (_queryPool) {
            _vkTable.vkDestroyQueryPool(_device, _queryPool, getAllocCalls());
        }

        _commandBufferManager.destroy();

        _vkTable.vkDestroyDevice(_device, getAllocCalls());
    }

    if (_surface) {
        _vkTable.vkDestroySurfaceKHR(_instance, _surface, getAllocCalls());
    }

    if (_instance) {
        _vkTable.vkDestroyInstance(_instance, getAllocCalls());
    }
}

void Device::create(Application* application, gerium_uint32_t version, bool enableValidations) {
    _enableValidations = enableValidations;
    _enableDebugNames  = enableValidations;
    _application       = application;
    _logger            = Logger::create("gerium:renderer:vulkan");
    _logger->setLevel(enableValidations ? GERIUM_LOGGER_LEVEL_DEBUG : GERIUM_LOGGER_LEVEL_OFF);
    _application->getSize(&_appWidth, &_appHeight);

    createInstance(application->getTitle(), version);
    createSurface(application);
    createPhysicalDevice();
    createDevice();
    createProfiler(32);
    createDescriptorPool();
    createVmaAllocator();
    createDynamicBuffer();
    createDefaultSampler();
    createSynchronizations();
    createSwapchain(application);
    createImGui(application);
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
        _application->getSize(&_appWidth, &_appHeight);
        resizeSwapchain();
    } else {
        check(result);
    }

    if (_frameCommandBuffer) {
        _frameCommandBuffer->submit(QueueType::Graphics);
        _frameCommandBuffer = nullptr;
    }

    _dynamicAllocatedSize = _dynamicBufferSize * _currentFrame;
    _commandBufferManager.newFrame();

    if (_profilerEnabled) {
        _profiler->resetTimestamps();
    }

    ImGui_ImplVulkan_NewFrame();
    _application->newFrameImGui();
    ImGui::NewFrame();

    if (!_frameCommandBuffer) {
        _frameCommandBuffer = getCommandBuffer(0, false);
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

        enqueuedCommandBuffers[i] = commandBuffer->_commandBuffer;
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
    buffer->size         = creation.size;
    buffer->name         = intern(creation.name);
    buffer->parent       = Undefined;

    constexpr auto dynamicBufferFlags =
        GERIUM_BUFFER_USAGE_VERTEX | GERIUM_BUFFER_USAGE_INDEX | GERIUM_BUFFER_USAGE_UNIFORM;

    const bool useGlobalBuffer = gerium_uint32_t(creation.usageFlags & dynamicBufferFlags) != 0;
    if (creation.usage == ResourceUsageType::Dynamic && useGlobalBuffer) {
        buffer->parent = _dynamicBuffer;
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
                    error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
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

    if (creation.initialData) {
        VkMemoryPropertyFlags memPropFlags;
        vmaGetAllocationMemoryProperties(_vmaAllocator, buffer->vmaAllocation, &memPropFlags);

        if (memPropFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            if (buffer->mappedData) {
                memcpy(buffer->mappedData, creation.initialData, (size_t) creation.size);
            } else {
                void* data = mapBuffer(handle);
                memcpy(data, creation.initialData, (size_t) creation.size);
                unmapBuffer(handle);
            }
        } else {
            BufferCreation stagingCreation{};
            stagingCreation.set(dynamicBufferFlags, ResourceUsageType::Dynamic, buffer->size);
            auto stagingBuffer = createBuffer(stagingCreation);

            auto ptr = mapBuffer(stagingBuffer, 0, buffer->size);
            memcpy(ptr, creation.initialData, buffer->size);
            unmapBuffer(stagingBuffer);

            _frameCommandBuffer->copyBuffer(stagingBuffer, handle);
            destroyBuffer(stagingBuffer);
        }
    }

    return handle;
}

TextureHandle Device::createTexture(const TextureCreation& creation) {
    auto [handle, texture] = _textures.obtain_and_access();

    texture->vkFormat = toVkFormat(creation.format);
    texture->width    = creation.width;
    texture->height   = creation.height;
    texture->depth    = creation.depth;
    texture->mipmaps  = creation.mipmaps;
    texture->flags    = creation.flags;
    texture->type     = creation.type;
    texture->name     = intern(creation.name);
    texture->sampler  = Undefined;

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
    imageInfo.imageType             = toVkImageType(creation.type);
    imageInfo.format                = texture->vkFormat;
    imageInfo.extent.width          = creation.width;
    imageInfo.extent.height         = creation.height;
    imageInfo.extent.depth          = creation.depth;
    imageInfo.mipLevels             = creation.mipmaps;
    imageInfo.arrayLayers           = 1;
    imageInfo.samples               = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling                = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage                 = usage;
    imageInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.queueFamilyIndexCount = 0;
    imageInfo.pQueueFamilyIndices   = nullptr;
    imageInfo.initialLayout         = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo memoryInfo{};
    memoryInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (creation.alias == Undefined) { //  || !_imageAliasingSupported) {
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

    VkImageAspectFlags aspectMask{};
    if (hasDepthOrStencil(texture->vkFormat)) {
        aspectMask |= hasDepth(texture->vkFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
        aspectMask |= hasStencil(texture->vkFormat) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
    } else {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image      = texture->vkImage;
    viewInfo.viewType   = toVkImageViewType(creation.type);
    viewInfo.format     = texture->vkFormat;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY };

    viewInfo.subresourceRange.aspectMask     = aspectMask;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = creation.mipmaps;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    check(_vkTable.vkCreateImageView(_device, &viewInfo, getAllocCalls(), &texture->vkImageView));
    setObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t) texture->vkImageView, texture->name);

    if (creation.initialData) {
        // uploadTextureData(texture, creation.initialData);
    }

    return handle;
}

SamplerHandle Device::createSampler(const SamplerCreation& creation) {
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
    check(_vkTable.vkCreateSampler(_device, &createInfo, getAllocCalls(), &sampler->vkSampler));

    setObjectName(VK_OBJECT_TYPE_SAMPLER, (uint64_t) sampler->vkSampler, sampler->name);

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
    createInfo.layers          = 1;
    check(_vkTable.vkCreateFramebuffer(_device, &createInfo, getAllocCalls(), &framebuffer->vkFramebuffer));

    setObjectName(VK_OBJECT_TYPE_FRAMEBUFFER, (uint64_t) framebuffer->vkFramebuffer, framebuffer->name);

    return handle;
}

DescriptorSetHandle Device::createDescriptorSet(const DescriptorSetCreation& creation) {
    auto [handle, descriptorSet] = _descriptorSets.obtain_and_access();

    for (auto& binding : descriptorSet->bindings) {
        binding = Undefined;
    }
    descriptorSet->layout = Undefined;
    descriptorSet->dirty  = true;

    // auto descriptorSetLayout = _descriptorSetLayouts.access(creation.layout);

    // VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    // allocInfo.descriptorPool     = _descriptorPool;
    // allocInfo.descriptorSetCount = 1;
    // allocInfo.pSetLayouts        = &descriptorSetLayout->vkDescriptorSetLayout;

    // check(_vkTable.vkAllocateDescriptorSets(_device, &allocInfo, &descriptorSet->vkDescriptorSet));

    // descriptorSet->numResources = creation.numResources;
    // descriptorSet->layout       = creation.layout;

    // for (uint32_t i = 0; i < creation.numResources; ++i) {
    //     descriptorSet->resources[i] = creation.resources[i];
    //     descriptorSet->samplers[i]  = creation.samplers[i];
    //     descriptorSet->bindings[i]  = creation.bindings[i];
    // }

    // VkWriteDescriptorSet descriptorWrite[kMaxDescriptorsPerSet]{};
    // VkDescriptorBufferInfo bufferInfo[kMaxDescriptorsPerSet]{};
    // VkDescriptorImageInfo imageInfo[kMaxDescriptorsPerSet]{};

    // const uint32_t num =
    //     fillWriteDescriptorSets(*descriptorSetLayout, *descriptorSet, descriptorWrite, bufferInfo, imageInfo);

    // _vkTable.vkUpdateDescriptorSets(_device, num, descriptorWrite, 0, nullptr);

    return handle;
}

DescriptorSetLayoutHandle Device::createDescriptorSetLayout(const DescriptorSetLayoutCreation& creation) {
    auto [handle, descriptorSetLayout] = _descriptorSetLayouts.obtain_and_access();

    descriptorSetLayout->data = *creation.setLayout;

    check(_vkTable.vkCreateDescriptorSetLayout(
        _device, &creation.setLayout->createInfo, getAllocCalls(), &descriptorSetLayout->vkDescriptorSetLayout));
    return handle;
}

ProgramHandle Device::createProgram(const ProgramCreation& creation) {
    auto [handle, program] = _programs.obtain_and_access();

    // VkPipelineShaderStageCreateInfo shaderStageInfo[kMaxShaderStages];
    program->name             = intern(creation.name);
    program->graphicsPipeline = true;

    std::map<uint32_t, std::set<uint32_t>> uniqueBindings;

    for (; program->activeShaders < creation.stagesCount; ++program->activeShaders) {
        const auto& stage = creation.stages[program->activeShaders];

        if (stage.type == VK_SHADER_STAGE_COMPUTE_BIT) {
            program->graphicsPipeline = false;
        }

        VkShaderModuleCreateInfo shaderInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };

        std::vector<uint32_t> code;
        if (creation.spvInput) {
            shaderInfo.codeSize = stage.codeSize;
            shaderInfo.pCode    = reinterpret_cast<const uint32_t*>(stage.code);
        } else {
            code                = compileGLSL(stage.code, stage.codeSize, stage.type, creation.name);
            shaderInfo.codeSize = code.size() * 4;
            shaderInfo.pCode    = code.data();
        }

        VkPipelineShaderStageCreateInfo& shaderStageInfo = program->shaderStageInfo[program->activeShaders];
        shaderStageInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.pName                            = "main";
        shaderStageInfo.stage                            = stage.type;
        check(_vkTable.vkCreateShaderModule(
            _device, &shaderInfo, getAllocCalls(), &program->shaderStageInfo[program->activeShaders].module));

        SpvReflectShaderModule module{};
        if (spvReflectCreateShaderModule(code.size() * 4, (void*) code.data(), &module) != SPV_REFLECT_RESULT_SUCCESS) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        uint32_t count = 0;
        if (spvReflectEnumerateDescriptorSets(&module, &count, NULL) != SPV_REFLECT_RESULT_SUCCESS) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        std::vector<SpvReflectDescriptorSet*> sets(count);
        if (spvReflectEnumerateDescriptorSets(&module, &count, sets.data()) != SPV_REFLECT_RESULT_SUCCESS) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        for (uint32_t set = 0; set < sets.size(); ++set) {
            const SpvReflectDescriptorSet& reflSet = *(sets[set]);

            DescriptorSetLayoutData& layout = program->descriptorSets[reflSet.set];
            auto& uniqueBinding             = uniqueBindings[reflSet.set];

            for (uint32_t binding = 0; binding < reflSet.binding_count; ++binding) {
                const SpvReflectDescriptorBinding& reflBinding = *(reflSet.bindings[binding]);
                layout.bindings.push_back({});
                VkDescriptorSetLayoutBinding& layoutBinding = layout.bindings.back();
                layoutBinding.binding                       = reflBinding.binding;
                layoutBinding.descriptorType  = static_cast<VkDescriptorType>(reflBinding.descriptor_type);
                layoutBinding.descriptorCount = 1;
                if (layoutBinding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                }
                for (uint32_t iDim = 0; iDim < reflBinding.array.dims_count; ++iDim) {
                    layoutBinding.descriptorCount *= reflBinding.array.dims[iDim];
                }
                layoutBinding.stageFlags = static_cast<VkShaderStageFlagBits>(module.shader_stage);

                if (uniqueBinding.contains(reflBinding.binding)) {
                    error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
                }
                uniqueBinding.insert(reflBinding.binding);
            }
            layout.setNumber               = reflSet.set;
            layout.createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layout.createInfo.bindingCount = layout.bindings.size();
            layout.createInfo.pBindings    = layout.bindings.data();
        }

        setObjectName(VK_OBJECT_TYPE_SHADER_MODULE,
                      (uint64_t) program->shaderStageInfo[program->activeShaders].module,
                      creation.name);
    }

    return handle;
}

PipelineHandle Device::createPipeline(const PipelineCreation& creation) {
    auto [handle, pipeline] = _pipelines.obtain_and_access();

    VkPipelineCacheCreateInfo cacheInfo{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };

    auto cacheExists = false;

    if (cacheExists) {
        // auto binary = foundation::fileRead(cachePath);

        // VkPipelineCacheHeaderVersionOne* header = (VkPipelineCacheHeaderVersionOne*) binary.data();

        // if (binary.size() >= sizeof(VkPipelineCacheHeaderVersionOne) &&
        //     header->deviceID == _deviceProperties.deviceID && header->vendorID == _deviceProperties.vendorID &&
        //     memcmp(header->pipelineCacheUUID, _deviceProperties.pipelineCacheUUID, VK_UUID_SIZE) == 0) {
        //     cacheInfo.initialDataSize = binary.size();
        //     cacheInfo.pInitialData    = binary.data();
        // } else {
        //     cacheExists = false;
        // }
    }

    VkPipelineCache pipelineCache;
    check(_vkTable.vkCreatePipelineCache(_device, &cacheInfo, getAllocCalls(), &pipelineCache));

    auto programHandle = createProgram(creation.program);
    auto program       = _programs.access(programHandle);

    pipeline->program = programHandle;

    VkDescriptorSetLayout vkLayouts[kMaxDescriptorSetLayouts];
    uint32_t numActiveLayouts = 0; // shader.parseResult->setCount;

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

    // Create full pipeline
    if (program->graphicsPipeline) {
        VkPipelineVertexInputStateCreateInfo vertexInput{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        VkVertexInputAttributeDescription vertexAttributes[kMaxVertexAttributes];
        VkVertexInputBindingDescription vertexBindings[kMaxVertexStreams];

        if (creation.vertexInput.numVertexAttributes) {
            for (uint32_t i = 0; i < creation.vertexInput.numVertexAttributes; ++i) {
                const auto& vertexAttribute = creation.vertexInput.vertexAttributes[i];

                vertexAttributes[i] = { vertexAttribute.location,
                                        vertexAttribute.binding,
                                        toVkVertexFormat(vertexAttribute.format),
                                        vertexAttribute.offset };
            }
            vertexInput.vertexAttributeDescriptionCount = creation.vertexInput.numVertexAttributes;
            vertexInput.pVertexAttributeDescriptions    = vertexAttributes;
        } else {
            vertexInput.vertexAttributeDescriptionCount = 0;
            vertexInput.pVertexAttributeDescriptions    = nullptr;
        }

        if (creation.vertexInput.numVertexStreams) {
            vertexInput.vertexBindingDescriptionCount = creation.vertexInput.numVertexStreams;

            for (uint32_t i = 0; i < creation.vertexInput.numVertexStreams; ++i) {
                const auto& vertex_stream = creation.vertexInput.vertexStreams[i];

                VkVertexInputRate vertex_rate = vertex_stream.inputRate == VertexInputRate::PerVertex
                                                    ? VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX
                                                    : VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE;

                vertexBindings[i] = { vertex_stream.binding, vertex_stream.stride, vertex_rate };
            }
            vertexInput.pVertexBindingDescriptions = vertexBindings;
        } else {
            vertexInput.vertexBindingDescriptionCount = 0;
            vertexInput.pVertexBindingDescriptions    = nullptr;
        }

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO
        };
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
        rasterizer.depthClampEnable        = creation.rasterization->depth_clamp_enable;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode             = toVkPolygonMode(creation.rasterization->polygon_mode);
        rasterizer.lineWidth               = creation.rasterization->line_width;
        rasterizer.cullMode                = toVkCullMode(creation.rasterization->cull_mode);
        rasterizer.frontFace               = toVkFrontFace(creation.rasterization->front_face);
        rasterizer.depthBiasEnable         = creation.rasterization->depth_bias_enable;
        rasterizer.depthBiasConstantFactor = creation.rasterization->depth_bias_constant_factor;
        rasterizer.depthBiasClamp          = creation.rasterization->depth_bias_clamp;
        rasterizer.depthBiasSlopeFactor    = creation.rasterization->depth_bias_slope_factor;

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
        depthStencil.depthTestEnable       = creation.depthStencil->depth_test_enable;
        depthStencil.depthWriteEnable      = creation.depthStencil->depth_write_enable;
        depthStencil.depthCompareOp        = toVkCompareOp(creation.depthStencil->depth_compare_op);
        depthStencil.depthBoundsTestEnable = creation.depthStencil->depth_bounds_test_enable;
        depthStencil.stencilTestEnable     = creation.depthStencil->stencil_test_enable;
        depthStencil.front.failOp          = toVkStencilOp(creation.depthStencil->front.fail_op);
        depthStencil.front.passOp          = toVkStencilOp(creation.depthStencil->front.pass_op);
        depthStencil.front.depthFailOp     = toVkStencilOp(creation.depthStencil->front.depth_fail_op);
        depthStencil.front.compareOp       = toVkCompareOp(creation.depthStencil->front.compare_op);
        depthStencil.front.compareMask     = creation.depthStencil->front.compare_mask;
        depthStencil.front.writeMask       = creation.depthStencil->front.write_mask;
        depthStencil.front.reference       = creation.depthStencil->front.reference;
        depthStencil.back.failOp           = toVkStencilOp(creation.depthStencil->back.fail_op);
        depthStencil.back.passOp           = toVkStencilOp(creation.depthStencil->back.pass_op);
        depthStencil.back.depthFailOp      = toVkStencilOp(creation.depthStencil->back.depth_fail_op);
        depthStencil.back.compareOp        = toVkCompareOp(creation.depthStencil->back.compare_op);
        depthStencil.back.compareMask      = creation.depthStencil->back.compare_mask;
        depthStencil.back.writeMask        = creation.depthStencil->back.write_mask;
        depthStencil.back.reference        = creation.depthStencil->back.reference;
        depthStencil.minDepthBounds        = creation.depthStencil->min_depth_bounds;
        depthStencil.maxDepthBounds        = creation.depthStencil->max_depth_bounds;

        VkPipelineColorBlendAttachmentState colorBlendAttachment[kMaxImageOutputs];
        if (creation.blendState.activeStates) {
            assert(creation.blendState.activeStates == creation.renderPass.numColorFormats);
            for (uint32_t i = 0; i < creation.blendState.activeStates; i++) {
                const BlendState& blendState = creation.blendState.blendStates[i];

                colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment[i].blendEnable         = blendState.blendEnabled ? VK_TRUE : VK_FALSE;
                colorBlendAttachment[i].srcColorBlendFactor = blendState.sourceColor;
                colorBlendAttachment[i].dstColorBlendFactor = blendState.destinationColor;
                colorBlendAttachment[i].colorBlendOp        = blendState.colorOperation;

                if (blendState.separateBlend) {
                    colorBlendAttachment[i].srcAlphaBlendFactor = blendState.sourceAlpha;
                    colorBlendAttachment[i].dstAlphaBlendFactor = blendState.destinationAlpha;
                    colorBlendAttachment[i].alphaBlendOp        = blendState.alphaOperation;
                } else {
                    colorBlendAttachment[i].srcAlphaBlendFactor = blendState.sourceColor;
                    colorBlendAttachment[i].dstAlphaBlendFactor = blendState.destinationColor;
                    colorBlendAttachment[i].alphaBlendOp        = blendState.colorOperation;
                }
            }
        } else {
            for (uint32_t i = 0; i < creation.renderPass.numColorFormats; ++i) {
                colorBlendAttachment[i]                = {};
                colorBlendAttachment[i].blendEnable    = VK_FALSE;
                colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
        }

        VkPipelineColorBlendStateCreateInfo colorBlending{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp       = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount =
            creation.blendState.activeStates ? creation.blendState.activeStates : creation.renderPass.numColorFormats;
        colorBlending.pAttachments      = colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

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

        //// Render Pass
        //  VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info{
        //     VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR};
        //  if (dynamic_rendering_extension_present) {
        //      pipeline_rendering_create_info.viewMask             = 0;
        //      pipeline_rendering_create_info.colorAttachmentCount = creation.render_pass.num_color_formats;
        //      pipeline_rendering_create_info.pColorAttachmentFormats =
        //          creation.render_pass.num_color_formats > 0 ? creation.render_pass.color_formats : nullptr;
        //      pipeline_rendering_create_info.depthAttachmentFormat   = creation.render_pass.depth_stencil_format;
        //      pipeline_rendering_create_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
        //      pipeline_info.pNext = &pipeline_rendering_create_info;
        //  } else {

        RenderPassCreation rc{};
        rc.output               = creation.renderPass;
        rc.name                 = creation.name;
        pipeline->renderPass    = createRenderPass(rc);
        pipelineInfo.renderPass = _renderPasses.access(pipeline->renderPass)->vkRenderPass;
        //}

        check(_vkTable.vkCreateGraphicsPipelines(
            _device, pipelineCache, 1, &pipelineInfo, getAllocCalls(), &pipeline->vkPipeline));

        pipeline->vkBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    } else {
        VkComputePipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
        pipelineInfo.stage  = program->shaderStageInfo[0];
        pipelineInfo.layout = pipelineLayout;

        check(_vkTable.vkCreateComputePipelines(
            _device, pipelineCache, 1, &pipelineInfo, getAllocCalls(), &pipeline->vkPipeline));

        pipeline->renderPass  = Undefined;
        pipeline->vkBindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    // if (cachePath != nullptr && !cacheExists) {
    //     size_t cacheSize = 0;
    //     check(_vkTable.vkGetPipelineCacheData(_device, pipelineCache, &cacheSize, nullptr));

    //     std::vector<uint8_t> cacheData;
    //     cacheData.resize(cacheSize);
    //     check(_vkTable.vkGetPipelineCacheData(_device, pipelineCache, &cacheSize, (void*) cacheData.data()));

    //     foundation::fileWrite(cachePath, (void*) cacheData.data(), cacheData.size());
    // }

    _vkTable.vkDestroyPipelineCache(_device, pipelineCache, getAllocCalls());

    // for (uint32_t i = 0; i < shader.activeShaders; ++i) {
    //     _vkTable.vkDestroyShaderModule(_device, shader.shaderStageInfo[i].module, getAllocCalls());
    //     shader.shaderStageInfo[i].module = VK_NULL_HANDLE;
    // }

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

    if (buffer->parent == _dynamicBuffer) {
        buffer->globalOffset = _dynamicAllocatedSize;

        uint8_t* mappedMemory = _dynamicBufferMapped + _dynamicAllocatedSize;
        _dynamicAllocatedSize += align(size ? size : buffer->size, _uboAlignment);

        return (void*) (mappedMemory + offset);
    }

    void* data = nullptr;
    check(vmaMapMemory(_vmaAllocator, buffer->vmaAllocation, &data));
    return (uint8_t*) data + offset;
}

void Device::unmapBuffer(BufferHandle handle) {
    auto buffer = _buffers.access(handle);

    if (buffer->parent == _dynamicBuffer) {
        auto parentBuffer = _buffers.access(buffer->parent);
        vmaFlushAllocation(_vmaAllocator, parentBuffer->vmaAllocation, buffer->globalOffset, buffer->size);
        return;
    }

    vmaUnmapMemory(_vmaAllocator, buffer->vmaAllocation);
}

void Device::bind(DescriptorSetHandle handle, uint16_t binding, Handle resource) {
    auto descriptorSet = _descriptorSets.access(handle);

    if (descriptorSet->bindings[binding] != resource) {
        descriptorSet->bindings[binding] = resource;
        descriptorSet->dirty             = true;
    }
}

void Device::updateDescriptorSet(DescriptorSetHandle handle, DescriptorSetLayoutHandle layoutHandle) {
    auto descriptorSet = _descriptorSets.access(handle);

    if (descriptorSet->dirty) {
        descriptorSet->dirty = false;

        auto layout              = _descriptorSetLayouts.access(layoutHandle);
        bool createDescriptorSet = descriptorSet->layout == Undefined;

        if (!createDescriptorSet && descriptorSet->layout != layoutHandle) {
            auto descriptorLayout = _descriptorSetLayouts.access(descriptorSet->layout);
            // createDescriptorSet = descriptorLayout->data.bindings != pipelineLayout->data.bindings;
        }

        descriptorSet->currentFrame = (descriptorSet->currentFrame + 1) % MaxFrames;

        auto& currestSet = descriptorSet->vkDescriptorSet[descriptorSet->currentFrame];

        createDescriptorSet = currestSet == VK_NULL_HANDLE ? true : createDescriptorSet;

        if (createDescriptorSet) {
            VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
            allocInfo.descriptorPool     = _descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts        = &layout->vkDescriptorSetLayout;

            check(_vkTable.vkAllocateDescriptorSets(_device, &allocInfo, &currestSet));
            descriptorSet->layout = layoutHandle;
        }

        // descriptorSet->numResources = creation.numResources;
        // descriptorSet->layout       = creation.layout;

        // for (uint32_t i = 0; i < creation.numResources; ++i) {
        //     descriptorSet->resources[i] = creation.resources[i];
        //     descriptorSet->samplers[i]  = creation.samplers[i];
        //     descriptorSet->bindings[i]  = creation.bindings[i];
        // }

        VkWriteDescriptorSet descriptorWrite[kMaxDescriptorsPerSet]{};
        VkDescriptorBufferInfo bufferInfo[kMaxDescriptorsPerSet]{};
        VkDescriptorImageInfo imageInfo[kMaxDescriptorsPerSet]{};

        const uint32_t num = fillWriteDescriptorSets(*layout, *descriptorSet, descriptorWrite, bufferInfo, imageInfo);

        _vkTable.vkUpdateDescriptorSets(_device, num, descriptorWrite, 0, nullptr);
    }
}

CommandBuffer* Device::getCommandBuffer(uint32_t thread, bool profile) {
    return _commandBufferManager.getCommandBuffer(_currentFrame, thread, profile);
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
#ifndef __APPLE__
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

    _deviceProperties2Supported = std::find_if(extensions.cbegin(), extensions.cend(), [](const auto& extension) {
        return strcmp(extension, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0;
    }) != std::end(extensions);

    if (layers.empty()) {
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
    appInfo.pEngineName        = "vision-flow";
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
    createInfo.pNext                   = layers.empty() ? nullptr : &features;
    createInfo.flags                   = 0;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = (uint32_t) layers.size();
    createInfo.ppEnabledLayerNames     = layers.data();
    createInfo.enabledExtensionCount   = (uint32_t) extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef __APPLE__
    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

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
    _uboAlignment  = (uint32_t) _deviceProperties.limits.minUniformBufferOffsetAlignment;
    _ssboAlignment = (uint32_t) _deviceProperties.limits.minStorageBufferOffsetAlignment;

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

void Device::createDevice() {
    const float priorities[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    const auto layers        = selectValidationLayers();
    const auto extensions    = selectDeviceExtensions(_physicalDevice);

    _memoryBudgetSupported = std::find_if(extensions.cbegin(), extensions.cend(), [](const auto extension) {
        return strcmp(extension, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0;
    }) != std::end(extensions);

    // VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV aliasingFeatures{
    //     VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV
    // };
    // VkPhysicalDeviceFeatures2 features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
    // features.pNext = (void*) &aliasingFeatures;

    // if (_deviceProperties2Supported) {
    //     _vkTable.vkGetPhysicalDeviceFeatures2(_physicalDevice, &features);
    //     _imageAliasingSupported = aliasingFeatures.dedicatedAllocationImageAliasing;
    // }

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

    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.pNext                   = nullptr; // _deviceProperties2Supported ? (void*) &features : nullptr;
    createInfo.queueCreateInfoCount    = (uint32_t) queueCreateInfoCount;
    createInfo.pQueueCreateInfos       = queueCreateInfos;
    createInfo.enabledLayerCount       = (uint32_t) layers.size();
    createInfo.ppEnabledLayerNames     = layers.data();
    createInfo.enabledExtensionCount   = (uint32_t) extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.pEnabledFeatures        = nullptr;

    check(_vkTable.vkCreateDevice(_physicalDevice, &createInfo, getAllocCalls(), &_device));
    _vkTable.init(vk::Device(_device));

    _vkTable.vkGetDeviceQueue(_device, graphic.index, graphic.queue, &_queueGraphic);
    _vkTable.vkGetDeviceQueue(_device, compute.index, compute.queue, &_queueCompute);
    _vkTable.vkGetDeviceQueue(_device, present.index, present.queue, &_queuePresent);
    _vkTable.vkGetDeviceQueue(_device, transfer.index, transfer.queue, &_queueTransfer);

    _commandBufferManager.create(*this, 4, graphic.index);
    _frameCommandBuffer = getCommandBuffer(0, false);
}

void Device::createProfiler(uint16_t gpuTimeQueriesPerFrame) {
    if (_profilerSupported) {
        VkProfiler* profiler;
        Object::create<VkProfiler>(profiler, *this, gpuTimeQueriesPerFrame, MaxFrames);
        _profiler = profiler;
        profiler->destroy();

        VkQueryPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
        createInfo.queryType          = VK_QUERY_TYPE_TIMESTAMP;
        createInfo.queryCount         = gpuTimeQueriesPerFrame * 2 * MaxFrames;
        createInfo.pipelineStatistics = 0;
        check(_vkTable.vkCreateQueryPool(_device, &createInfo, getAllocCalls(), &_queryPool));

        _gpuFrequency = _deviceProperties.limits.timestampPeriod / (1'000'000.0);
    }
}

void Device::createDescriptorPool() {
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kGlobalPoolElements * 2 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          kGlobalPoolElements     },
        //{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   kGlobalPoolElements     },
        //{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, kGlobalPoolElements     },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       kGlobalPoolElements     }
    };

    VkDescriptorPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets       = kDescriptorSetsPoolSize;
    poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
    poolInfo.pPoolSizes    = poolSizes;
    check(_vkTable.vkCreateDescriptorPool(_device, &poolInfo, getAllocCalls(), &_descriptorPool));
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

void Device::createDynamicBuffer() {
    _dynamicBufferSize = 1024 * 1024 * 10;

    BufferCreation bc;
    bc.set(GERIUM_BUFFER_USAGE_VERTEX | GERIUM_BUFFER_USAGE_INDEX | GERIUM_BUFFER_USAGE_UNIFORM,
           ResourceUsageType::Staging,
           _dynamicBufferSize * MaxFrames)
        .setPersistent(true)
        .setName("Dynamic_Persistent_Buffer");
    _dynamicBuffer       = createBuffer(bc);
    _dynamicBufferMapped = (uint8_t*) _buffers.access(_dynamicBuffer)->mappedData;
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

void Device::createSynchronizations() {
    VkSemaphoreCreateInfo semaphoreInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VkFenceCreateInfo fenceInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < MaxFrames; ++i) {
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

    const auto imageCount = std::clamp(3U, swapchain.capabilities.minImageCount, swapchain.capabilities.maxImageCount);

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
        rc.output.color(_swapchainFormat.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, GERIUM_RENDER_PASS_OPERATION_CLEAR);
        rc.output.depth(VK_FORMAT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        rc.output.setDepthStencilOperations(GERIUM_RENDER_PASS_OPERATION_CLEAR, GERIUM_RENDER_PASS_OPERATION_CLEAR);
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

    auto commandBuffer = getCommandBuffer(0, false);

    for (uint32_t i = 0; i < swapchainImages; ++i) {
        auto [colorHandle, color] = _textures.obtain_and_access();

        color->vkImage = images[i];

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

        commandBuffer->addImageBarrier(
            colorHandle, ResourceState::Undefined, ResourceState::Present, 0, 1, false, false);
    }

    commandBuffer->submit(QueueType::Graphics);
}

void Device::createImGui(Application* application) {
    auto renderPass = _renderPasses.access(_swapchainRenderPass)->vkRenderPass;

    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
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
    initInfo.MinImageCount       = _swapchainFramebuffers.size();
    initInfo.ImageCount          = _swapchainFramebuffers.size();
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
    io.Fonts->AddFontFromMemoryTTF(dataFont, font.size(), fontSize * density, &config);
    ImGui::GetStyle().ScaleAllSizes(density);
    ImGui_ImplVulkan_CreateFontsTexture();
}

void Device::resizeSwapchain() {
    _vkTable.vkDeviceWaitIdle(_device);

    auto oldSwapchain = _swapchain;
    createSwapchain(_application);

    if (oldSwapchain) {
        _vkTable.vkDestroySwapchainKHR(_device, oldSwapchain, getAllocCalls());
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

uint32_t Device::fillWriteDescriptorSets(const DescriptorSetLayout& descriptorSetLayout,
                                         const DescriptorSet& descriptorSet,
                                         VkWriteDescriptorSet* descriptorWrite,
                                         VkDescriptorBufferInfo* bufferInfo,
                                         VkDescriptorImageInfo* imageInfo) {
    uint32_t usedResources = 0;

    auto defaultSampler = _samplers.access(_defaultSampler)->vkSampler;

    for (uint32_t b = 0; b < kMaxDescriptorsPerSet; ++b) {
        auto resource = descriptorSet.bindings[b];

        if (resource == Undefined) {
            continue;
        }

        auto it = std::find_if(descriptorSetLayout.data.bindings.cbegin(),
                               descriptorSetLayout.data.bindings.cend(),
                               [b](const auto& binding) {
            return binding.binding == b;
        });

        if (it == descriptorSetLayout.data.bindings.cend()) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        const auto& binding = *it;

        auto i = usedResources++;

        descriptorWrite[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[i].dstSet          = descriptorSet.vkDescriptorSet[descriptorSet.currentFrame];
        descriptorWrite[i].dstBinding      = b;
        descriptorWrite[i].dstArrayElement = 0;
        descriptorWrite[i].descriptorCount = binding.descriptorCount;

        switch (binding.descriptorType) {
            case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                auto texture = _textures.access(resource);

                imageInfo[i].sampler =
                    texture->sampler != Undefined ? _samplers.access(texture->sampler)->vkSampler : defaultSampler;

                // if (descriptorSet.samplers[r] != Undefined) {
                //     auto sampler         = _samplers.access(descriptorSet.samplers[r]);
                //     imageInfo[i].sampler = sampler->vkSampler;
                // }

                imageInfo[i].imageLayout = hasDepthOrStencil(texture->vkFormat)
                                               ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
                                               : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                imageInfo[i].imageView = texture->vkImageView;

                descriptorWrite[i].pImageInfo = &imageInfo[i];
                break;
            }
            case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
                descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

                auto texture = _textures.access(resource);

                imageInfo[i].sampler     = VK_NULL_HANDLE;
                imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
                imageInfo[i].imageView   = texture->vkImageView;

                descriptorWrite[i].pImageInfo = &imageInfo[i];
                break;
            }
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
                auto buffer = _buffers.access(resource);

                descriptorWrite[i].descriptorType = buffer->usage == ResourceUsageType::Dynamic
                                                        ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
                                                        : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

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
            case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
                descriptorWrite[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

                auto buffer = _buffers.access(resource);

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
    }

    return usedResources;
}

std::vector<uint32_t> Device::compileGLSL(const char* code,
                                          size_t size,
                                          VkShaderStageFlagBits stage,
                                          const char* name) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    shaderc_shader_kind kind;

    switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT:
            options.AddMacroDefinition("VERTEX"s, "1"s);
            kind = shaderc_glsl_vertex_shader;
            break;
        case VK_SHADER_STAGE_FRAGMENT_BIT:
            options.AddMacroDefinition("FRAGMENT"s, "1"s);
            kind = shaderc_glsl_fragment_shader;
            break;
        case VK_SHADER_STAGE_COMPUTE_BIT:
            options.AddMacroDefinition("COMPUTE"s, "1"s);
            kind = shaderc_glsl_compute_shader;
            break;
        default:
            throw std::runtime_error("Not supported shader type");
    }

    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    options.SetWarningsAsErrors();
    options.SetPreserveBindings(true);
    options.SetAutoMapLocations(true);
    options.SetAutoBindUniforms(true);
    options.SetAutoSampledTextures(true);

    auto result = compiler.CompileGlslToSpv(code, size, kind, name, "main", options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::stringstream ss(result.GetErrorMessage());
        std::string message;
        while (std::getline(ss, message, '\n')) {
            _logger->print(GERIUM_LOGGER_LEVEL_ERROR, message.c_str());
        }
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add error type;
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
        case GERIUM_RENDER_PASS_OPERATION_DONT_CARE:
            depthOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthInitial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        case GERIUM_RENDER_PASS_OPERATION_LOAD:
            depthOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case GERIUM_RENDER_PASS_OPERATION_CLEAR:
            depthOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthInitial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        default:
            assert(!"unreachable code");
            break;
    }

    switch (output.stencilOperation) {
        case GERIUM_RENDER_PASS_OPERATION_DONT_CARE:
            stencilOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        case GERIUM_RENDER_PASS_OPERATION_LOAD:
            stencilOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case GERIUM_RENDER_PASS_OPERATION_CLEAR:
            stencilOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            break;
        default:
            assert(!"unreachable code");
            break;
    }

    uint32_t attachmentCount = 0;
    for (; attachmentCount < output.numColorFormats; ++attachmentCount) {
        VkAttachmentLoadOp colorOp;
        VkImageLayout colorInitial;
        switch (output.colorOperations[attachmentCount]) {
            case GERIUM_RENDER_PASS_OPERATION_DONT_CARE:
                colorOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorInitial = VK_IMAGE_LAYOUT_UNDEFINED;
                break;
            case GERIUM_RENDER_PASS_OPERATION_LOAD:
                colorOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            case GERIUM_RENDER_PASS_OPERATION_CLEAR:
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
    }

    if (output.depthStencilFormat != VK_FORMAT_UNDEFINED) {
        auto& depthAttachment          = attachmets[attachmentCount];
        depthAttachment.format         = output.depthStencilFormat;
        depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp         = depthOp;
        depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp  = stencilOp;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout  = depthInitial;
        depthAttachment.finalLayout    = output.depthStencilFinalLayout;

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
    for (; dependencyCount < output.numColorFormats; ++dependencyCount) {
        auto& dependency      = dependencies[dependencyCount];
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
    }

    if (output.depthStencilFormat != VK_FORMAT_UNDEFINED) {
        auto& dependency      = dependencies[dependencyCount++];
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
    }

    VkRenderPassCreateInfo createInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    createInfo.attachmentCount = attachmentCount;
    createInfo.pAttachments    = attachmets;
    createInfo.subpassCount    = 1;
    createInfo.pSubpasses      = &subpass;

    VkRenderPass renderPass;
    check(_vkTable.vkCreateRenderPass(_device, &createInfo, getAllocCalls(), &renderPass));

    setObjectName(VK_OBJECT_TYPE_RENDER_PASS, (uint64_t) renderPass, name);

    return renderPass;
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
                    if (texture->vkImageView) {
                        _vkTable.vkDestroyImageView(_device, texture->vkImageView, getAllocCalls());
                    }
                    if (texture->vkImage && texture->vmaAllocation) {
                        vmaDestroyImage(_vmaAllocator, texture->vkImage, texture->vmaAllocation);
                    }
                }
                _textures.release(resource.handle);
                break;
            case ResourceType::Sampler:
                if (_samplers.references(SamplerHandle{ resource.handle }) == 1) {
                    auto sampler = _samplers.access(resource.handle);
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
                    auto descriptorSet = _descriptorSets.access(resource.handle); // ???????????
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
                    destroyProgram(pipeline->program);
                    destroyRenderPass(pipeline->renderPass);
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

    try {
        selectDeviceExtensions(device);
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
    QueueFamilies result;

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
        result.transfer = result.graphic;
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
    _currentFrame  = (_currentFrame + 1) % MaxFrames;
    ++_absoluteFrame;
}

std::vector<const char*> Device::selectValidationLayers() {
    return checkValidationLayers({ "VK_LAYER_KHRONOS_validation", "MoltenVK" });
}

std::vector<const char*> Device::selectExtensions() {
    std::vector<std::pair<const char*, bool>> extensions = {
        { VK_KHR_SURFACE_EXTENSION_NAME,                          true  },
        { onGetSurfaceExtension(),                                true  },
        { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, false }
    };

    if (_enableValidations) {
        extensions.push_back({ VK_EXT_DEBUG_UTILS_EXTENSION_NAME, false });
    }

    return checkExtensions(extensions);
}

std::vector<const char*> Device::selectDeviceExtensions(VkPhysicalDevice device) {
    std::vector<std::pair<const char*, bool>> extensions = {
        { VK_KHR_SWAPCHAIN_EXTENSION_NAME,     true  },
        { VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, false },
    // { VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME, false },
#ifdef __APPLE__
        { "VK_KHR_portability_subset",         true  },
#endif
    };

    return checkDeviceExtensions(device, extensions);
}

VkPhysicalDevice Device::selectPhysicalDevice() {
    uint32_t count = 0;
    check(_vkTable.vkEnumeratePhysicalDevices(_instance, &count, nullptr));

    if (!count) {
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, "failed to find GPUs with Vulkan API support");
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
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
    error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err

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
                const auto found = std::find(results.cbegin(), results.cend(), layer) != results.cend();
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
        const auto notFound = std::find(results.cbegin(), results.cend(), extension) == results.cend();
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

} // namespace gerium::vulkan
