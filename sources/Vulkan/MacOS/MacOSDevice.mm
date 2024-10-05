#include "MacOSDevice.hpp"
#include "../../MacOS/MacOSApplication.hpp"

namespace gerium::vulkan::macos {

VkInstanceCreateFlags MacOSDevice::onGetCreateInfoFlags() const noexcept {
    return VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
}

const void* MacOSDevice::onGetNextCreateInfo(void* pNext) noexcept {
    static int32_t useMetalArgumentBuffers = 1;
    static VkLayerSettingEXT layerSetting{};
    layerSetting.pLayerName   = "MoltenVK";
    layerSetting.pSettingName = "MVK_CONFIG_USE_METAL_ARGUMENT_BUFFERS";
    layerSetting.type         = VK_LAYER_SETTING_TYPE_INT32_EXT;
    layerSetting.valueCount   = 1;
    layerSetting.pValues      = &useMetalArgumentBuffers;

    static VkLayerSettingsCreateInfoEXT layerSettingsCreateInfo{ VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT };
    layerSettingsCreateInfo.pNext        = pNext,
    layerSettingsCreateInfo.settingCount = 1,
    layerSettingsCreateInfo.pSettings    = &layerSetting;

    return &layerSettingsCreateInfo;
}

std::vector<const char*> MacOSDevice::onGetInstanceExtensions() const noexcept {
    return { VK_EXT_LAYER_SETTINGS_EXTENSION_NAME, VK_EXT_METAL_SURFACE_EXTENSION_NAME };
}

std::vector<const char*> MacOSDevice::onGetDeviceExtensions() const noexcept {
    return { "VK_KHR_portability_subset" };
}

bool MacOSDevice::onNeedPostAcquireResize() const noexcept {
    return true;
}

VkSurfaceKHR MacOSDevice::onCreateSurface(Application* application) const {
    auto macosApp = alias_cast<gerium::macos::MacOSApplication*>(application);

    VkMetalSurfaceCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT };
    createInfo.pLayer = macosApp->layer();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    check(vkTable().vkCreateMetalSurfaceEXT(instance(), &createInfo, getAllocCalls(), &surface));
    return surface;
}

} // namespace gerium::vulkan::macos
