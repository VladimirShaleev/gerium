#include "Device.hpp"

using namespace std::string_view_literals;

namespace gerium::vulkan {

Device::~Device() {
    if (_device) {
        _vkTable.vkDeviceWaitIdle(_device);

        for (auto renderPass : _renderPasses) {
            destroyRenderPass(renderPass.handle);
        }
        
        for (auto texture : _textures) {
            destroyTexture(texture.handle);
        }

        deleteResources(true);

        if (_swapchain) {
            _vkTable.vkDestroySwapchainKHR(_device, _swapchain, getAllocCalls());
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
    _logger            = Logger::create("gerium:renderer:vulkan");
    _logger->setLevel(enableValidations ? GERIUM_LOGGER_LEVEL_DEBUG : GERIUM_LOGGER_LEVEL_OFF);

    createInstance(application->getTitle(), version);
    createSurface(application);
    createPhysicalDevice();
    createDevice();
    createVmaAllocator();
    createSwapchain(application);
}

TextureHandle Device::createTexture(const TextureCreation& creation) {
    auto [handle, texture] = _textures.obtain_and_access();

    texture.vkFormat = toVkFormat(creation.format);
    texture.width    = creation.width;
    texture.height   = creation.height;
    texture.depth    = creation.depth;
    texture.mipmaps  = creation.mipmaps;
    texture.flags    = creation.flags;
    texture.type     = creation.type;
    texture.name     = intern(creation.name);
    texture.handle   = handle;

    VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;

    if ((creation.flags & TextureFlags::Compute) == TextureFlags::Compute) {
        usage |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    if (hasDepthOrStencil(texture.vkFormat)) {
        usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    } else {
        const auto renderTarget = (creation.flags & TextureFlags::RenderTarget) == TextureFlags::RenderTarget;
        usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        usage |= renderTarget ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : 0;
    }

    VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageInfo.imageType             = toVkImageType(creation.type);
    imageInfo.format                = texture.vkFormat;
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
        check(
            vmaCreateImage(_vmaAllocator, &imageInfo, &memoryInfo, &texture.vkImage, &texture.vmaAllocation, nullptr));

        if (_enableValidations && texture.name) {
            vmaSetAllocationName(_vmaAllocator, texture.vmaAllocation, texture.name);
        }
    } else {
        auto& aliasTexture = _textures.access(creation.alias);
        check(vmaCreateAliasingImage(_vmaAllocator, aliasTexture.vmaAllocation, &imageInfo, &texture.vkImage));
    }

    setObjectName(VK_OBJECT_TYPE_IMAGE, (uint64_t) texture.vkImage, texture.name);

    VkImageAspectFlags aspectMask;
    if (hasDepthOrStencil(texture.vkFormat)) {
        aspectMask = hasDepth(texture.vkFormat) ? VK_IMAGE_ASPECT_DEPTH_BIT : 0;
    } else {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    VkImageViewCreateInfo viewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewInfo.image      = texture.vkImage;
    viewInfo.viewType   = toVkImageViewType(creation.type);
    viewInfo.format     = texture.vkFormat;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY,
                            VK_COMPONENT_SWIZZLE_IDENTITY };

    viewInfo.subresourceRange.aspectMask     = aspectMask;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = creation.mipmaps;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    check(_vkTable.vkCreateImageView(_device, &viewInfo, getAllocCalls(), &texture.vkImageView));
    setObjectName(VK_OBJECT_TYPE_IMAGE_VIEW, (uint64_t) texture.vkImageView, texture.name);

    if (creation.initialData) {
        // uploadTextureData(texture, creation.initialData);
    }

    return handle;
}

RenderPassHandle Device::createRenderPass(const RenderPassCreation& creation) {
    const auto key = hash(creation.output);
    if (auto it = _renderPassCache.find(key); it != _renderPassCache.end()) {
        _renderPasses.access(it->second).addReference();
        return it->second;
    }

    auto [handle, renderPass] = _renderPasses.obtain_and_access();

    renderPass.handle       = handle;
    renderPass.references   = 1;
    renderPass.output       = creation.output;
    renderPass.name         = intern(creation.name);
    renderPass.vkRenderPass = vkCreateRenderPass(renderPass.output, renderPass.name);

    _renderPassCache[key] = handle;
    return handle;
}

void Device::destroyTexture(TextureHandle handle) {
    _deletionQueue.push({ ResourceType::Texture, 0 /* _currentFrame */, handle });
}

void Device::destroyRenderPass(RenderPassHandle handle) {
    _deletionQueue.push({ ResourceType::RenderPass, 0 /* _currentFrame */, handle });
}

CommandBuffer* Device::getCommandBuffer(uint32_t thread, bool profile) {
    auto commandBuffer = _commandBufferManager.getCommandBuffer(0 /* _currentFrame */, thread);
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
        if (resource.frame == 0 /* _currentFrame */ || forceDelete) {
            switch (resource.type) {
                case ResourceType::Buffer:
                    break;
                case ResourceType::Texture: {
                    auto& texture = _textures.access(resource.handle);
                    if (texture.vkImageView) {
                        _vkTable.vkDestroyImageView(_device, texture.vkImageView, getAllocCalls());
                    }
                    if (texture.vkImage && texture.vmaAllocation) {
                        vmaDestroyImage(_vmaAllocator, texture.vkImage, texture.vmaAllocation);
                    }
                    _textures.release(resource.handle);
                    break;
                }
                case ResourceType::Sampler:
                    break;
                case ResourceType::RenderPass: {
                    auto& renderPass = _renderPasses.access(resource.handle);
                    if (renderPass.removeReference() == 0) {
                        _vkTable.vkDestroyRenderPass(_device, renderPass.vkRenderPass, getAllocCalls());
                        _renderPasses.release(resource.handle);
                    }
                    break;
                }
                case ResourceType::Framebuffer:
                    break;
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
