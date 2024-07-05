#include "Device.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace gerium::vulkan {

Device::~Device() {
    if (_device) {
        _vkTable.vkDeviceWaitIdle(_device);
        deleteResources(true);

        for (auto framebuffer : _framebuffers) {
            destroyFramebuffer(_framebuffers.handle(framebuffer));
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
    _application       = application;
    _logger            = Logger::create("gerium:renderer:vulkan");
    _logger->setLevel(enableValidations ? GERIUM_LOGGER_LEVEL_DEBUG : GERIUM_LOGGER_LEVEL_OFF);
    _application->getSize(&_appWidth, &_appHeight);

    createInstance(application->getTitle(), version);
    createSurface(application);
    createPhysicalDevice();
    createDevice();
    createVmaAllocator();
    createSynchronizations();
    createSwapchain(application);

    const char vs[] = "#version 450\n"
                      "\n"
                      "vec2 positions[3] = vec2[](\n"
                      "    vec2(0.0, -0.5),\n"
                      "    vec2(0.5, 0.5),\n"
                      "    vec2(-0.5, 0.5)\n"
                      ");\n"
                      "\n"
                      "void main() {\n"
                      "    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);"
                      "}\n";

    const auto vsSize = sizeof(vs) - 1;

    const char fs[] = "#version 450\n"
                      "\n"
                      "layout(location = 0) out vec4 outColor;\n"
                      "\n"
                      "void main() {\n"
                      "    outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
                      "}\n";

    const auto fsSize = sizeof(fs) - 1;

    auto shaderVs = compileGLSL(vs, vsSize, VK_SHADER_STAGE_VERTEX_BIT, "shader.vs.glsl");
    auto shaderFs = compileGLSL(fs, fsSize, VK_SHADER_STAGE_FRAGMENT_BIT, "shader.fs.glsl");
}

void Device::newFrame() {
    constexpr auto max = std::numeric_limits<uint64_t>::max();

    check(_vkTable.vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, max));
    check(_vkTable.vkResetFences(_device, 1, &_inFlightFences[_currentFrame]));

    const auto result = _vkTable.vkAcquireNextImageKHR(_device,
                                                       _swapchain,
                                                       std::numeric_limits<uint64_t>::max(),
                                                       _imageAvailableSemaphores[_currentFrame],
                                                       nullptr,
                                                       &_swapchainImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        resizeSwapchain();
    } else {
        check(result);
    }

    //_dynamicAllocatedSize = _dynamicBufferSize * _currentFrame;
    _commandBufferManager.newFrame();

    if (_profilerEnabled) {
        // _timestampManager.reset();
    }
}

void Device::submit(CommandBuffer* commandBuffer) {
    assert(_numQueuedCommandBuffers < sizeof(_queuedCommandBuffers) / sizeof(_queuedCommandBuffers[0]));
    _queuedCommandBuffers[_numQueuedCommandBuffers++] = commandBuffer;
}

void Device::present() {
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
        // if (_timestampManager.hasQueries()) {
        //     const auto queryOffset = _currentFrame * _timestampManager.getQueriesPerFrame() * 2;
        //     const auto queryCount  = _timestampManager.getCurrentQuery() * 2;

        //     check(_vkTable.vkGetQueryPoolResults(_device,
        //                                          _queryPool,
        //                                          queryOffset,
        //                                          queryCount,
        //                                          sizeof(uint64_t) * queryCount * 2,
        //                                          _timestampManager.getTimestampData(queryOffset),
        //                                          sizeof(uint64_t),
        //                                          VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

        //     for (uint32_t i = 0; i < _timestampManager.getCurrentQuery(); ++i) {
        //         auto index = _currentFrame * _timestampManager.getQueriesPerFrame() + i;

        //         auto& timestamp = _timestampManager.getTimestamp(index);

        //         double start       = (double) *_timestampManager.getTimestampData(index * 2);
        //         double end         = (double) *_timestampManager.getTimestampData(index * 2 + 1);
        //         double range       = end - start;
        //         double elapsedTime = range * _gpuFrequency;

        //         timestamp.elapsedMs  = elapsedTime;
        //         timestamp.frameIndex = _absoluteFrame;
        //     }
        // }

        // _profilerReset = true;
    } else {
        // _profilerReset = false;
    }

    gerium_uint16_t appWidth, appHeight;
    _application->getSize(&appWidth, &appHeight);
    const auto resized = _appWidth != appWidth || _appHeight != appHeight;

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || resized) {
        _appWidth  = appWidth;
        _appHeight = appHeight;
        resizeSwapchain();
        frameCountersAdvance();
        return;
    }

    check(result);

    frameCountersAdvance();
    deleteResources();
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

    if (creation.alias == Undefined) {
        check(vmaCreateImage(
            _vmaAllocator, &imageInfo, &memoryInfo, &texture->vkImage, &texture->vmaAllocation, nullptr));

        if (_enableValidations && texture->name) {
            vmaSetAllocationName(_vmaAllocator, texture->vmaAllocation, texture->name);
        }
    } else {
        auto aliasTexture = _textures.access(creation.alias);
        check(vmaCreateAliasingImage(_vmaAllocator, aliasTexture->vmaAllocation, &imageInfo, &texture->vkImage));
    }

    setObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t) texture->vkImage, texture->name);

    VkImageAspectFlags aspectMask;
    if (hasDepthOrStencil(texture->vkFormat)) {
        aspectMask = hasDepth(texture->vkFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
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

DescriptorSetLayoutHandle Device::createDescriptorSetLayout(const DescriptorSetLayoutCreation& creation) {
    auto [handle, DescriptorSetLayout] = _descriptorSetLayouts.obtain_and_access();

    return handle;
}

ProgramHandle Device::createProgram(const ProgramCreation& creation) {
    auto [handle, program] = _programs.obtain_and_access();

    // VkPipelineShaderStageCreateInfo shaderStageInfo[kMaxShaderStages];
    program->name             = intern(creation.name);
    program->graphicsPipeline = true;

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

        // spirv::parseBinary(shaderInfo.pCode, shaderInfo.codeSize, shader.parseResult);

        setObjectName(VK_OBJECT_TYPE_SHADER_MODULE,
                      (uint64_t) program->shaderStageInfo[program->activeShaders].module,
                      creation.name);
    }

    return handle;
}

PipelineHandle Device::createPipeline(const PipelineCreation& creation) {
    auto [handle, pipelines] = _pipelines.obtain_and_access();

    return handle;
}

void Device::destroyTexture(TextureHandle handle) {
    _deletionQueue.push({ ResourceType::Texture, _currentFrame, handle });
}

void Device::destroyRenderPass(RenderPassHandle handle) {
    _deletionQueue.push({ ResourceType::RenderPass, _currentFrame, handle });
}

void Device::destroyFramebuffer(FramebufferHandle handle) {
    _deletionQueue.push({ ResourceType::Framebuffer, _currentFrame, handle });
}

CommandBuffer* Device::getCommandBuffer(uint32_t thread, bool profile) {
    auto commandBuffer = _commandBufferManager.getCommandBuffer(_currentFrame, thread);
    if (_profilerEnabled) {
        // TODO:
    }
    return commandBuffer;
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
    createInfo.pNext                   = _enableValidations ? &features : nullptr;
    createInfo.flags                   = 0;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledLayerCount       = (uint32_t) layers.size();
    createInfo.ppEnabledLayerNames     = layers.data();
    createInfo.enabledExtensionCount   = (uint32_t) extensions.size();
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef VK_USE_PLATFORM_MACOS_MVK
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
    const auto extensions    = selectDeviceExtensions();

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
}

void Device::createVmaAllocator() {
    VmaVulkanFunctions functions{};
    functions.vkGetInstanceProcAddr = _vkTable.vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr   = _vkTable.vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo createInfo{};
    createInfo.physicalDevice       = _physicalDevice;
    createInfo.device               = _device;
    createInfo.instance             = _instance;
    createInfo.pAllocationCallbacks = getAllocCalls();
    createInfo.pVulkanFunctions     = &functions;

    check(vmaCreateAllocator(&createInfo, &_vmaAllocator));
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
        rc.output.color(_swapchainFormat.format, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, RenderPassOperation::Clear);
        rc.output.depth(VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        rc.output.setDepthStencilOperations(RenderPassOperation::Clear, RenderPassOperation::Clear);
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

    auto commandBuffer = getCommandBuffer(0);

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

        TextureCreation depthCreation{};
        depthCreation.setName("SwapchainDepthStencilTexture");
        depthCreation.setSize(_swapchainExtent.width, _swapchainExtent.height, 1);
        depthCreation.setFlags(1, false, false);
        depthCreation.setFormat(GERIUM_FORMAT_D32_SFLOAT, GERIUM_TEXTURE_TYPE_2D);

        auto depthHandle = createTexture(depthCreation);
        auto depth       = _textures.access(depthHandle);

        auto name = "SwapchainFramebuffer"s + std::to_string(i);

        FramebufferCreation fc;
        fc.setName(name.c_str());
        fc.setScaling(1.0f, 1.0f, 0);
        fc.addRenderTexture(colorHandle);
        fc.setDepthStencilTexture(depthHandle);
        fc.width      = _swapchainExtent.width;
        fc.height     = _swapchainExtent.height;
        fc.renderPass = _swapchainRenderPass;

        _swapchainFramebuffers[i] = createFramebuffer(fc);

        _textures.release(colorHandle);
        _textures.release(depthHandle);

        commandBuffer->addImageBarrier(colorHandle, ResourceState::Undefined, ResourceState::Present, 0, 1, false);
    }

    commandBuffer->submit(QueueType::Graphics);
}

void Device::resizeSwapchain() {
    _vkTable.vkDeviceWaitIdle(_device);

    auto oldSwapchain = _swapchain;
    createSwapchain(_application);

    if (oldSwapchain) {
        _vkTable.vkDestroySwapchainKHR(_device, oldSwapchain, getAllocCalls());
    }

    if (_profilerEnabled) {
        // _profilerReset = true;
    }
}

void Device::printValidationLayers() {
    if (_enableValidations) {
        uint32_t count = 0;
        check(_vkTable.vkEnumerateInstanceLayerProperties(&count, nullptr));

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
        case RenderPassOperation::DontCare:
            depthOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthInitial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        case RenderPassOperation::Load:
            depthOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
            depthInitial = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
        case RenderPassOperation::Clear:
            depthOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthInitial = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        default:
            assert(!"unreachable code");
            break;
    }

    switch (output.stencilOperation) {
        case RenderPassOperation::DontCare:
            stencilOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            break;
        case RenderPassOperation::Load:
            stencilOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            break;
        case RenderPassOperation::Clear:
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
            case RenderPassOperation::DontCare:
                colorOp      = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorInitial = VK_IMAGE_LAYOUT_UNDEFINED;
                break;
            case RenderPassOperation::Load:
                colorOp      = VK_ATTACHMENT_LOAD_OP_LOAD;
                colorInitial = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            case RenderPassOperation::Clear:
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
                break;
            case ResourceType::Texture: {
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
            }
            case ResourceType::Sampler:
                break;
            case ResourceType::RenderPass: {
                if (_renderPasses.references(RenderPassHandle{ resource.handle }) == 1) {
                    auto renderPass = _renderPasses.access(resource.handle);
                    _renderPassCache.erase(hash(renderPass->output));
                    _vkTable.vkDestroyRenderPass(_device, renderPass->vkRenderPass, getAllocCalls());
                }
                _renderPasses.release(resource.handle);
                break;
            }
            case ResourceType::Framebuffer: {
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
            }
            case ResourceType::Shader:
                break;
            case ResourceType::DescriptorSet:
                break;
            case ResourceType::DescriptorSetLayout:
                break;
            case ResourceType::Pipeline:
                break;
        }
        _deletionQueue.pop();
    }
}

void Device::setObjectName(VkObjectType type, uint64_t handle, gerium_utf8_t name) {
    if (_enableValidations && name) {
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

    if (!checkPhysicalDeviceExtensions(device, selectDeviceExtensions())) {
        return 0;
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
    std::vector extensions = { VK_KHR_SURFACE_EXTENSION_NAME, onGetSurfaceExtension() };

    if (_enableValidations) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return checkExtensions(extensions);
}

std::vector<const char*> Device::selectDeviceExtensions() {
    return {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_MACOS_MVK
        "VK_KHR_portability_subset",
#endif
    };
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
        props.resize(count);
        check(_vkTable.vkEnumerateInstanceLayerProperties(&count, props.data()));

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

std::vector<const char*> Device::checkExtensions(const std::vector<const char*>& extensions) {
    std::vector<const char*> results;

    uint32_t count = 0;
    check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));

    std::vector<VkExtensionProperties> props;
    props.resize(count);
    check(_vkTable.vkEnumerateInstanceExtensionProperties(nullptr, &count, props.data()));

    for (const auto& prop : props) {
        for (const auto& extension : extensions) {
            if (strncmp(prop.extensionName, extension, 255) == 0) {
                results.push_back(extension);
                break;
            }
        }
    }

    for (const auto& extension : extensions) {
        const auto notFound = std::find(results.cbegin(), results.cend(), extension) == results.cend();
        if (notFound) {
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

void Device::error(gerium_result_t result) {
    throw Exception(result);
}

} // namespace gerium::vulkan
