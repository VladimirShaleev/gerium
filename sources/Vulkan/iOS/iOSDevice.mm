#include "iOSDevice.hpp"
#include "../../iOS/iOSApplication.hpp"

namespace gerium::vulkan::ios {

VkInstanceCreateFlags iOSDevice::onGetCreateInfoFlags() const noexcept {
    return VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
}

const void* iOSDevice::onGetNextCreateInfo(void* pNext) noexcept {
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

std::vector<const char*> iOSDevice::onGetInstanceExtensions() const noexcept {
    return { VK_EXT_LAYER_SETTINGS_EXTENSION_NAME, VK_EXT_METAL_SURFACE_EXTENSION_NAME };
}

std::vector<const char*> iOSDevice::onGetDeviceExtensions() const noexcept {
    return { "VK_KHR_portability_subset" };
}

bool iOSDevice::onNeedPostAcquireResize() const noexcept {
    return true; // TODO
}

VkSurfaceKHR iOSDevice::onCreateSurface(Application* application) const {
    auto iosApp = alias_cast<gerium::ios::iOSApplication*>(application);

    VkMetalSurfaceCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT };
    // createInfo.pLayer = iosApp->layer();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    check(vkTable().vkCreateMetalSurfaceEXT(instance(), &createInfo, getAllocCalls(), &surface));
    return surface;
}

} // namespace gerium::vulkan::ios
