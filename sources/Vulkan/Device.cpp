#include "Device.hpp"

using namespace std::string_view_literals;

namespace gerium::vulkan {

Device::~Device() {
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

    if (_logger->getLevel() == GERIUM_LOGGER_LEVEL_VERBOSE) {
        _logger->setLevel(enableValidations ? GERIUM_LOGGER_LEVEL_DEBUG : GERIUM_LOGGER_LEVEL_OFF);
    }

    createInstance(application->getTitle(), version);
    createSurface(application);
    createPhysicalDevice();
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
    createInfo.enabledLayerCount       = layers.size();
    createInfo.ppEnabledLayerNames     = layers.data();
    createInfo.enabledExtensionCount   = extensions.size();
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
    _uboAlignment  = _deviceProperties.limits.minUniformBufferOffsetAlignment;
    _ssboAlignment = _deviceProperties.limits.minStorageBufferOffsetAlignment;

    _profilerSupported =
        _deviceProperties.limits.timestampPeriod != 0 && _deviceProperties.limits.timestampComputeAndGraphics;
    if (_profilerSupported) {
        _profilerSupported = _queueFamilies.graphic.value().timestampValidBits != 0 &&
                             _queueFamilies.compute.value().timestampValidBits != 0;
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
