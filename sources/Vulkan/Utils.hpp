#ifndef GERIUM_WINDOWS_VULKAN_UTILS_HPP
#define GERIUM_WINDOWS_VULKAN_UTILS_HPP

#include "../Gerium.hpp"

namespace gerium::vulkan {

void check(VkResult result);

const VkAllocationCallbacks* getAllocCalls() noexcept;

gerium_inline bool hasDepthOrStencil(VkFormat format) noexcept {
    return format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D24_UNORM_S8_UINT;
}

gerium_inline bool hasStencil(VkFormat format) noexcept {
    return format >= VK_FORMAT_S8_UINT && format <= VK_FORMAT_D32_SFLOAT_S8_UINT;
}

gerium_inline bool hasDepth(VkFormat format) noexcept {
    return (format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT) ||
           (format >= VK_FORMAT_D16_UNORM_S8_UINT && format <= VK_FORMAT_D32_SFLOAT_S8_UINT);
}

gerium_inline VkFormat toVkFormat(gerium_format_t format) noexcept {
    switch (format) {
        case GERIUM_FORMAT_R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case GERIUM_FORMAT_R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case GERIUM_FORMAT_R8_UINT:
            return VK_FORMAT_R8_UINT;
        case GERIUM_FORMAT_R8_SINT:
            return VK_FORMAT_R8_SINT;
        case GERIUM_FORMAT_R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case GERIUM_FORMAT_R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case GERIUM_FORMAT_R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case GERIUM_FORMAT_R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case GERIUM_FORMAT_R8G8B8_UNORM:
            return VK_FORMAT_R8G8B8_UNORM;
        case GERIUM_FORMAT_R8G8B8_SNORM:
            return VK_FORMAT_R8G8B8_SNORM;
        case GERIUM_FORMAT_R8G8B8_UINT:
            return VK_FORMAT_R8G8B8_UINT;
        case GERIUM_FORMAT_R8G8B8_SINT:
            return VK_FORMAT_R8G8B8_SINT;
        case GERIUM_FORMAT_R8G8B8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case GERIUM_FORMAT_R4G4B4A4_UNORM:
            return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case GERIUM_FORMAT_R5G5B5A1_UNORM:
            return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case GERIUM_FORMAT_R8G8B8A8_UNORM:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case GERIUM_FORMAT_R8G8B8A8_SNORM:
            return VK_FORMAT_R8G8B8A8_SNORM;
        case GERIUM_FORMAT_R8G8B8A8_UINT:
            return VK_FORMAT_R8G8B8A8_UINT;
        case GERIUM_FORMAT_R8G8B8A8_SINT:
            return VK_FORMAT_R8G8B8A8_SINT;
        case GERIUM_FORMAT_R8G8B8A8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case GERIUM_FORMAT_A2R10G10B10_UNORM:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_UINT:
            return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case GERIUM_FORMAT_R16_UINT:
            return VK_FORMAT_R16_UINT;
        case GERIUM_FORMAT_R16_SINT:
            return VK_FORMAT_R16_SINT;
        case GERIUM_FORMAT_R16_SFLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case GERIUM_FORMAT_R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case GERIUM_FORMAT_R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case GERIUM_FORMAT_R16G16_SFLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case GERIUM_FORMAT_R16G16B16_UINT:
            return VK_FORMAT_R16G16B16_UINT;
        case GERIUM_FORMAT_R16G16B16_SINT:
            return VK_FORMAT_R16G16B16_SINT;
        case GERIUM_FORMAT_R16G16B16_SFLOAT:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case GERIUM_FORMAT_R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case GERIUM_FORMAT_R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case GERIUM_FORMAT_R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case GERIUM_FORMAT_R32_UINT:
            return VK_FORMAT_R32_UINT;
        case GERIUM_FORMAT_R32_SINT:
            return VK_FORMAT_R32_SINT;
        case GERIUM_FORMAT_R32_SFLOAT:
            return VK_FORMAT_R32_SFLOAT;
        case GERIUM_FORMAT_R32G32_UINT:
            return VK_FORMAT_R32G32_UINT;
        case GERIUM_FORMAT_R32G32_SINT:
            return VK_FORMAT_R32G32_SINT;
        case GERIUM_FORMAT_R32G32_SFLOAT:
            return VK_FORMAT_R32G32_SFLOAT;
        case GERIUM_FORMAT_R32G32B32_UINT:
            return VK_FORMAT_R32G32B32_UINT;
        case GERIUM_FORMAT_R32G32B32_SINT:
            return VK_FORMAT_R32G32B32_SINT;
        case GERIUM_FORMAT_R32G32B32_SFLOAT:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case GERIUM_FORMAT_R32G32B32A32_UINT:
            return VK_FORMAT_R32G32B32A32_UINT;
        case GERIUM_FORMAT_R32G32B32A32_SINT:
            return VK_FORMAT_R32G32B32A32_SINT;
        case GERIUM_FORMAT_R32G32B32A32_SFLOAT:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case GERIUM_FORMAT_B10G11R11_UFLOAT:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case GERIUM_FORMAT_E5B9G9R9_UFLOAT:
            return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case GERIUM_FORMAT_D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case GERIUM_FORMAT_X8_D24_UNORM:
            return VK_FORMAT_X8_D24_UNORM_PACK32;
        case GERIUM_FORMAT_D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case GERIUM_FORMAT_S8_UINT:
            return VK_FORMAT_S8_UINT;
        case GERIUM_FORMAT_D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case GERIUM_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        default:
            assert(!"unreachable code");
            return VK_FORMAT_UNDEFINED;
    }
}

gerium_inline VkImageType toVkImageType(gerium_texture_type_t type) noexcept {
    switch (type) {
        case GERIUM_TEXTURE_TYPE_1D:
            return VK_IMAGE_TYPE_1D;
        case GERIUM_TEXTURE_TYPE_2D:
            return VK_IMAGE_TYPE_2D;
        case GERIUM_TEXTURE_TYPE_3D:
            return VK_IMAGE_TYPE_3D;
        case GERIUM_TEXTURE_TYPE_1D_ARRAY:
            return VK_IMAGE_TYPE_1D;
        case GERIUM_TEXTURE_TYPE_2D_ARRAY:
            return VK_IMAGE_TYPE_2D;
        case GERIUM_TEXTURE_TYPE_CUBE_ARRAY:
            return VK_IMAGE_TYPE_3D;
        default:
            assert(!"unreachable code");
            return {};
    }
}

gerium_inline VkImageViewType toVkImageViewType(gerium_texture_type_t type) noexcept {
    constexpr VkImageViewType types[] = { VK_IMAGE_VIEW_TYPE_1D,       VK_IMAGE_VIEW_TYPE_2D,
                                          VK_IMAGE_VIEW_TYPE_3D,       VK_IMAGE_VIEW_TYPE_1D_ARRAY,
                                          VK_IMAGE_VIEW_TYPE_2D_ARRAY, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY };
    return types[(int) type];
}

} // namespace gerium::vulkan

#endif
