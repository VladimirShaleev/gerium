#ifndef GERIUM_GERIUM_HPP
#define GERIUM_GERIUM_HPP

#define NOMINMAX
#define UNICODE
#define _UNICODE

#include "gerium/gerium-platform.h"
#include <locale>
#include <streambuf>
#if !defined(GERIUM_PLATFORM_ANDROID) && !defined(GERIUM_PLATFORM_LINUX)
# include <mimalloc-override.h>

namespace std { // add mi_* functions to std for vulkan headers

inline mi_decl_restrict void* mi_malloc(size_t size) mi_attr_noexcept mi_attr_alloc_size(1) {
    return mi_malloc(size);
}

inline void mi_free(void* p) mi_attr_noexcept {
    ::mi_free(p);
}

} // namespace std
#else
# define GERIUM_MIMALLOC_DISABLE
#endif

// cmrc
#include <cmrc/cmrc.hpp>
CMRC_DECLARE(gerium::resources);

#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

#define CTRE_STRING_IS_UTF8
#include <absl/container/flat_hash_map.h>
#include <ctre.hpp>
#include <rapidhash.h>

// Vulkan
#if defined(GERIUM_PLATFORM_WINDOWS)
# define VK_USE_PLATFORM_WIN32_KHR
#elif defined(GERIUM_PLATFORM_IOS)
# error unsupported platform
#elif defined(GERIUM_PLATFORM_MAC_OS)
# define VK_USE_PLATFORM_MACOS_MVK
# define VK_USE_PLATFORM_METAL_EXT
#elif defined(GERIUM_PLATFORM_ANDROID)
# define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(GERIUM_PLATFORM_LINUX)
# define VK_USE_PLATFORM_XCB_KHR
# define VK_USE_PLATFORM_WAYLAND_KHR
#else
# error unsupported platform
#endif
#ifndef __APPLE__
# define VK_NO_PROTOTYPES
# define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 1
#else
# define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 0
#endif
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_format_traits.hpp>

// VMA
#include <vk_mem_alloc.h>

#ifndef GERIUM_STATIC_BUILD
# ifdef _MSC_VER
#  define gerium_public __declspec(dllexport)
# elif __GNUC__ >= 4
#  define gerium_public __attribute__((visibility("default")))
# else
#  define gerium_public
# endif
#endif

#ifdef __STRICT_ANSI__
# undef inline
# define inline __inline__
#endif

#ifdef _MSC_VER
# define gerium_inline __forceinline
#else
# define gerium_inline inline __attribute__((always_inline))
#endif

// GLSL/HLSL compiler
#include <shaderc/shaderc.hpp>

// SPIRV-Reflect
#include <spirv-reflect/spirv_reflect.h>

// marl
#include "marl/defer.h"
#include "marl/event.h"
#include "marl/scheduler.h"
#include "marl/thread.h"
#include "marl/ticket.h"
#include "marl/waitgroup.h"

// GLM
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_MESSAGES
// https://github.com/g-truc/glm/issues/1269
#include <glm/detail/setup.hpp>
#undef GLM_DEPRECATED
#define GLM_DEPRECATED [[deprecated]]
#include <glm/ext.hpp>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_vulkan.h>

// Ktx
#include <ktxvulkan.h>

// Stb
#include <stb_image.h>

// FidelityFX
#include <FidelityFX/host/backends/vk/ffx_vk.h>

#include "gerium/gerium.h"

typedef ptrdiff_t gerium_sint_t;
typedef size_t gerium_uint_t;

namespace gerium {

template <typename AliasedType, typename Type>
gerium_inline AliasedType alias_cast(Type ptr) noexcept {
    static_assert(std::is_pointer_v<AliasedType>, "AliasedType must be a pointer");
    static_assert(std::is_pointer_v<Type>, "Type must be a pointer");
    AliasedType result;
    memcpy(&result, &ptr, sizeof(AliasedType));
    return result;
}

gerium_inline gerium_uint64_t hash(std::string_view str, gerium_uint64_t seed = RAPID_SEED) noexcept {
    return rapidhash_withSeed(str.data(), str.length(), seed);
}

gerium_inline gerium_uint64_t hash(gerium_utf8_t str, gerium_uint64_t seed = RAPID_SEED) noexcept {
    return rapidhash_withSeed(str, strlen(str), seed);
}

template <typename T>
gerium_inline gerium_uint64_t hash(const T& data, gerium_uint64_t seed = RAPID_SEED) noexcept {
    return rapidhash_withSeed(&data, sizeof(T), seed);
}

gerium_inline gerium_uint64_t hash(gerium_cdata_t data, gerium_uint32_t size, gerium_uint64_t seed = RAPID_SEED) noexcept {
    return rapidhash_withSeed(data, size, seed);
}

gerium_inline gerium_uint32_t align(gerium_uint32_t size, gerium_uint32_t alignment) noexcept {
    if (alignment) {
        const gerium_uint32_t mask = alignment - 1;
        return (size + mask) & ~mask;
    } else {
        return size;
    }
}

template <typename T>
gerium_inline gerium_uint8_t calcMipLevels(T width, T height) noexcept {
    static_assert(std::is_integral_v<T>, "T is not integral type");
    return static_cast<gerium_uint8_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

gerium_inline gerium_uint32_t formatBlockSize(gerium_format_t format) noexcept {
    switch (format) {
        case GERIUM_FORMAT_R4G4_UNORM:
            return 1;
        case GERIUM_FORMAT_R4G4B4A4_UNORM:
            return 2;
        case GERIUM_FORMAT_R5G5B5A1_UNORM:
            return 2;
        case GERIUM_FORMAT_R5G6B5_UNORM:
            return 2;
        case GERIUM_FORMAT_R8_UNORM:
            return 1;
        case GERIUM_FORMAT_R8_SNORM:
            return 1;
        case GERIUM_FORMAT_R8_UINT:
            return 1;
        case GERIUM_FORMAT_R8_SINT:
            return 1;
        case GERIUM_FORMAT_R8_SRGB:
            return 1;
        case GERIUM_FORMAT_R8_USCALED:
            return 1;
        case GERIUM_FORMAT_R8_SSCALED:
            return 1;
        case GERIUM_FORMAT_R8G8_UNORM:
            return 2;
        case GERIUM_FORMAT_R8G8_SNORM:
            return 2;
        case GERIUM_FORMAT_R8G8_UINT:
            return 2;
        case GERIUM_FORMAT_R8G8_SINT:
            return 2;
        case GERIUM_FORMAT_R8G8_SRGB:
            return 2;
        case GERIUM_FORMAT_R8G8_USCALED:
            return 2;
        case GERIUM_FORMAT_R8G8_SSCALED:
            return 2;
        case GERIUM_FORMAT_R8G8B8_UNORM:
            return 3;
        case GERIUM_FORMAT_R8G8B8_SNORM:
            return 3;
        case GERIUM_FORMAT_R8G8B8_UINT:
            return 3;
        case GERIUM_FORMAT_R8G8B8_SINT:
            return 3;
        case GERIUM_FORMAT_R8G8B8_SRGB:
            return 3;
        case GERIUM_FORMAT_R8G8B8_USCALED:
            return 3;
        case GERIUM_FORMAT_R8G8B8_SSCALED:
            return 3;
        case GERIUM_FORMAT_R8G8B8A8_UNORM:
            return 4;
        case GERIUM_FORMAT_R8G8B8A8_SNORM:
            return 4;
        case GERIUM_FORMAT_R8G8B8A8_UINT:
            return 4;
        case GERIUM_FORMAT_R8G8B8A8_SINT:
            return 4;
        case GERIUM_FORMAT_R8G8B8A8_SRGB:
            return 4;
        case GERIUM_FORMAT_R8G8B8A8_USCALED:
            return 4;
        case GERIUM_FORMAT_R8G8B8A8_SSCALED:
            return 4;
        case GERIUM_FORMAT_R16_UNORM:
            return 2;
        case GERIUM_FORMAT_R16_SNORM:
            return 2;
        case GERIUM_FORMAT_R16_UINT:
            return 2;
        case GERIUM_FORMAT_R16_SINT:
            return 2;
        case GERIUM_FORMAT_R16_SFLOAT:
            return 2;
        case GERIUM_FORMAT_R16_USCALED:
            return 2;
        case GERIUM_FORMAT_R16_SSCALED:
            return 2;
        case GERIUM_FORMAT_R16G16_UNORM:
            return 4;
        case GERIUM_FORMAT_R16G16_SNORM:
            return 4;
        case GERIUM_FORMAT_R16G16_UINT:
            return 4;
        case GERIUM_FORMAT_R16G16_SINT:
            return 4;
        case GERIUM_FORMAT_R16G16_SFLOAT:
            return 4;
        case GERIUM_FORMAT_R16G16_USCALED:
            return 4;
        case GERIUM_FORMAT_R16G16_SSCALED:
            return 4;
        case GERIUM_FORMAT_R16G16B16_UNORM:
            return 6;
        case GERIUM_FORMAT_R16G16B16_SNORM:
            return 6;
        case GERIUM_FORMAT_R16G16B16_UINT:
            return 6;
        case GERIUM_FORMAT_R16G16B16_SINT:
            return 6;
        case GERIUM_FORMAT_R16G16B16_SFLOAT:
            return 6;
        case GERIUM_FORMAT_R16G16B16_USCALED:
            return 6;
        case GERIUM_FORMAT_R16G16B16_SSCALED:
            return 6;
        case GERIUM_FORMAT_R16G16B16A16_UNORM:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_SNORM:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_UINT:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_SINT:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_SFLOAT:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_USCALED:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_SSCALED:
            return 8;
        case GERIUM_FORMAT_R32_UINT:
            return 4;
        case GERIUM_FORMAT_R32_SINT:
            return 4;
        case GERIUM_FORMAT_R32_SFLOAT:
            return 4;
        case GERIUM_FORMAT_R32G32_UINT:
            return 8;
        case GERIUM_FORMAT_R32G32_SINT:
            return 8;
        case GERIUM_FORMAT_R32G32_SFLOAT:
            return 8;
        case GERIUM_FORMAT_R32G32B32_UINT:
            return 12;
        case GERIUM_FORMAT_R32G32B32_SINT:
            return 12;
        case GERIUM_FORMAT_R32G32B32_SFLOAT:
            return 12;
        case GERIUM_FORMAT_R32G32B32A32_UINT:
            return 16;
        case GERIUM_FORMAT_R32G32B32A32_SINT:
            return 16;
        case GERIUM_FORMAT_R32G32B32A32_SFLOAT:
            return 16;
        case GERIUM_FORMAT_R64_UINT:
            return 8;
        case GERIUM_FORMAT_R64_SINT:
            return 8;
        case GERIUM_FORMAT_R64_SFLOAT:
            return 8;
        case GERIUM_FORMAT_R64G64_UINT:
            return 16;
        case GERIUM_FORMAT_R64G64_SINT:
            return 16;
        case GERIUM_FORMAT_R64G64_SFLOAT:
            return 16;
        case GERIUM_FORMAT_R64G64B64_UINT:
            return 24;
        case GERIUM_FORMAT_R64G64B64_SINT:
            return 24;
        case GERIUM_FORMAT_R64G64B64_SFLOAT:
            return 24;
        case GERIUM_FORMAT_R64G64B64A64_UINT:
            return 32;
        case GERIUM_FORMAT_R64G64B64A64_SINT:
            return 32;
        case GERIUM_FORMAT_R64G64B64A64_SFLOAT:
            return 32;
        case GERIUM_FORMAT_B4G4R4A4_UNORM:
            return 2;
        case GERIUM_FORMAT_B5G5R5A1_UNORM:
            return 2;
        case GERIUM_FORMAT_B5G6R5_UNORM:
            return 2;
        case GERIUM_FORMAT_B8G8R8_UNORM:
            return 3;
        case GERIUM_FORMAT_B8G8R8_SNORM:
            return 3;
        case GERIUM_FORMAT_B8G8R8_UINT:
            return 3;
        case GERIUM_FORMAT_B8G8R8_SINT:
            return 3;
        case GERIUM_FORMAT_B8G8R8_SRGB:
            return 3;
        case GERIUM_FORMAT_B8G8R8_USCALED:
            return 3;
        case GERIUM_FORMAT_B8G8R8_SSCALED:
            return 3;
        case GERIUM_FORMAT_B8G8R8A8_UNORM:
            return 4;
        case GERIUM_FORMAT_B8G8R8A8_SNORM:
            return 4;
        case GERIUM_FORMAT_B8G8R8A8_UINT:
            return 4;
        case GERIUM_FORMAT_B8G8R8A8_SINT:
            return 4;
        case GERIUM_FORMAT_B8G8R8A8_SRGB:
            return 4;
        case GERIUM_FORMAT_B8G8R8A8_USCALED:
            return 4;
        case GERIUM_FORMAT_B8G8R8A8_SSCALED:
            return 4;
        case GERIUM_FORMAT_B10G11R11_UFLOAT:
            return 4;
        case GERIUM_FORMAT_A1B5G5R5_UNORM:
            return 2;
        case GERIUM_FORMAT_A1R5G5B5_UNORM:
            return 2;
        case GERIUM_FORMAT_A2B10G10R10_UNORM:
            return 4;
        case GERIUM_FORMAT_A2B10G10R10_SNORM:
            return 4;
        case GERIUM_FORMAT_A2B10G10R10_UINT:
            return 4;
        case GERIUM_FORMAT_A2B10G10R10_SINT:
            return 4;
        case GERIUM_FORMAT_A2B10G10R10_USCALED:
            return 4;
        case GERIUM_FORMAT_A2B10G10R10_SSCALED:
            return 4;
        case GERIUM_FORMAT_A2R10G10B10_UNORM:
            return 4;
        case GERIUM_FORMAT_A2R10G10B10_SNORM:
            return 4;
        case GERIUM_FORMAT_A2R10G10B10_UINT:
            return 4;
        case GERIUM_FORMAT_A2R10G10B10_SINT:
            return 4;
        case GERIUM_FORMAT_A2R10G10B10_USCALED:
            return 4;
        case GERIUM_FORMAT_A2R10G10B10_SSCALED:
            return 4;
        case GERIUM_FORMAT_A4B4G4R4_UNORM:
            return 2;
        case GERIUM_FORMAT_A4R4G4B4_UNORM:
            return 2;
        case GERIUM_FORMAT_A8_UNORM:
            return 1;
        case GERIUM_FORMAT_A8B8G8R8_UNORM:
            return 4;
        case GERIUM_FORMAT_A8B8G8R8_SNORM:
            return 4;
        case GERIUM_FORMAT_A8B8G8R8_UINT:
            return 4;
        case GERIUM_FORMAT_A8B8G8R8_SINT:
            return 4;
        case GERIUM_FORMAT_A8B8G8R8_SRGB:
            return 4;
        case GERIUM_FORMAT_A8B8G8R8_USCALED:
            return 4;
        case GERIUM_FORMAT_A8B8G8R8_SSCALED:
            return 4;
        case GERIUM_FORMAT_S8_UINT:
            return 1;
        case GERIUM_FORMAT_X8_D24_UNORM:
            return 4;
        case GERIUM_FORMAT_D16_UNORM:
            return 2;
        case GERIUM_FORMAT_D16_UNORM_S8_UINT:
            return 3;
        case GERIUM_FORMAT_D24_UNORM_S8_UINT:
            return 4;
        case GERIUM_FORMAT_D32_SFLOAT:
            return 4;
        case GERIUM_FORMAT_D32_SFLOAT_S8_UINT:
            return 5;
        case GERIUM_FORMAT_ASTC_4x4_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_4x4_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_4x4_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_5x4_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_5x4_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_5x4_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_5x5_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_5x5_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_5x5_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_6x5_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_6x5_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_6x5_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_6x6_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_6x6_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_6x6_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_8x5_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_8x5_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_8x5_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_8x6_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_8x6_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_8x6_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_8x8_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_8x8_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_8x8_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_10x5_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_10x5_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_10x5_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_10x6_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_10x6_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_10x6_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_10x8_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_10x8_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_10x8_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_10x10_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_10x10_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_10x10_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_12x10_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_12x10_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_12x10_SFLOAT:
            return 16;
        case GERIUM_FORMAT_ASTC_12x12_UNORM:
            return 16;
        case GERIUM_FORMAT_ASTC_12x12_SRGB:
            return 16;
        case GERIUM_FORMAT_ASTC_12x12_SFLOAT:
            return 16;
        case GERIUM_FORMAT_BC1_RGB_UNORM:
            return 8;
        case GERIUM_FORMAT_BC1_RGB_SRGB:
            return 8;
        case GERIUM_FORMAT_BC1_RGBA_UNORM:
            return 8;
        case GERIUM_FORMAT_BC1_RGBA_SRGB:
            return 8;
        case GERIUM_FORMAT_BC2_UNORM:
            return 16;
        case GERIUM_FORMAT_BC2_SRGB:
            return 16;
        case GERIUM_FORMAT_BC3_UNORM:
            return 16;
        case GERIUM_FORMAT_BC3_SRGB:
            return 16;
        case GERIUM_FORMAT_BC4_UNORM:
            return 8;
        case GERIUM_FORMAT_BC4_SNORM:
            return 8;
        case GERIUM_FORMAT_BC5_UNORM:
            return 16;
        case GERIUM_FORMAT_BC5_SNORM:
            return 16;
        case GERIUM_FORMAT_BC6H_UFLOAT:
            return 16;
        case GERIUM_FORMAT_BC6H_SFLOAT:
            return 16;
        case GERIUM_FORMAT_BC7_UNORM:
            return 16;
        case GERIUM_FORMAT_BC7_SRGB:
            return 16;
        case GERIUM_FORMAT_ETC2_R8G8B8_UNORM:
            return 8;
        case GERIUM_FORMAT_ETC2_R8G8B8_SRGB:
            return 8;
        case GERIUM_FORMAT_ETC2_R8G8B8A1_UNORM:
            return 8;
        case GERIUM_FORMAT_ETC2_R8G8B8A1_SRGB:
            return 8;
        case GERIUM_FORMAT_ETC2_R8G8B8A8_UNORM:
            return 16;
        case GERIUM_FORMAT_ETC2_R8G8B8A8_SRGB:
            return 16;
        case GERIUM_FORMAT_EAC_R11_UNORM:
            return 8;
        case GERIUM_FORMAT_EAC_R11_SNORM:
            return 8;
        case GERIUM_FORMAT_EAC_R11G11_UNORM:
            return 16;
        case GERIUM_FORMAT_EAC_R11G11_SNORM:
            return 16;
        case GERIUM_FORMAT_PVRTC1_2BPP_UNORM:
            return 8;
        case GERIUM_FORMAT_PVRTC1_2BPP_SRGB:
            return 8;
        case GERIUM_FORMAT_PVRTC1_4BPP_UNORM:
            return 8;
        case GERIUM_FORMAT_PVRTC1_4BPP_SRGB:
            return 8;
        case GERIUM_FORMAT_PVRTC2_2BPP_UNORM:
            return 8;
        case GERIUM_FORMAT_PVRTC2_2BPP_SRGB:
            return 8;
        case GERIUM_FORMAT_PVRTC2_4BPP_UNORM:
            return 8;
        case GERIUM_FORMAT_PVRTC2_4BPP_SRGB:
            return 8;
        default:
            assert(!"unreachable code");
            return 0;
    }
}

gerium_inline VkFormat toVkFormat(gerium_format_t format) noexcept {
    switch (format) {
        case GERIUM_FORMAT_R4G4_UNORM:
            return VK_FORMAT_R4G4_UNORM_PACK8;
        case GERIUM_FORMAT_R4G4B4A4_UNORM:
            return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
        case GERIUM_FORMAT_R5G5B5A1_UNORM:
            return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
        case GERIUM_FORMAT_R5G6B5_UNORM:
            return VK_FORMAT_R5G6B5_UNORM_PACK16;
        case GERIUM_FORMAT_R8_UNORM:
            return VK_FORMAT_R8_UNORM;
        case GERIUM_FORMAT_R8_SNORM:
            return VK_FORMAT_R8_SNORM;
        case GERIUM_FORMAT_R8_UINT:
            return VK_FORMAT_R8_UINT;
        case GERIUM_FORMAT_R8_SINT:
            return VK_FORMAT_R8_SINT;
        case GERIUM_FORMAT_R8_SRGB:
            return VK_FORMAT_R8_SRGB;
        case GERIUM_FORMAT_R8_USCALED:
            return VK_FORMAT_R8_USCALED;
        case GERIUM_FORMAT_R8_SSCALED:
            return VK_FORMAT_R8_SSCALED;
        case GERIUM_FORMAT_R8G8_UNORM:
            return VK_FORMAT_R8G8_UNORM;
        case GERIUM_FORMAT_R8G8_SNORM:
            return VK_FORMAT_R8G8_SNORM;
        case GERIUM_FORMAT_R8G8_UINT:
            return VK_FORMAT_R8G8_UINT;
        case GERIUM_FORMAT_R8G8_SINT:
            return VK_FORMAT_R8G8_SINT;
        case GERIUM_FORMAT_R8G8_SRGB:
            return VK_FORMAT_R8G8_SRGB;
        case GERIUM_FORMAT_R8G8_USCALED:
            return VK_FORMAT_R8G8_USCALED;
        case GERIUM_FORMAT_R8G8_SSCALED:
            return VK_FORMAT_R8G8_SSCALED;
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
        case GERIUM_FORMAT_R8G8B8_USCALED:
            return VK_FORMAT_R8G8B8_USCALED;
        case GERIUM_FORMAT_R8G8B8_SSCALED:
            return VK_FORMAT_R8G8B8_SSCALED;
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
        case GERIUM_FORMAT_R8G8B8A8_USCALED:
            return VK_FORMAT_R8G8B8A8_USCALED;
        case GERIUM_FORMAT_R8G8B8A8_SSCALED:
            return VK_FORMAT_R8G8B8A8_SSCALED;
        case GERIUM_FORMAT_R16_UNORM:
            return VK_FORMAT_R16_UNORM;
        case GERIUM_FORMAT_R16_SNORM:
            return VK_FORMAT_R16_SNORM;
        case GERIUM_FORMAT_R16_UINT:
            return VK_FORMAT_R16_UINT;
        case GERIUM_FORMAT_R16_SINT:
            return VK_FORMAT_R16_SINT;
        case GERIUM_FORMAT_R16_SFLOAT:
            return VK_FORMAT_R16_SFLOAT;
        case GERIUM_FORMAT_R16_USCALED:
            return VK_FORMAT_R16_USCALED;
        case GERIUM_FORMAT_R16_SSCALED:
            return VK_FORMAT_R16_SSCALED;
        case GERIUM_FORMAT_R16G16_UNORM:
            return VK_FORMAT_R16G16_UNORM;
        case GERIUM_FORMAT_R16G16_SNORM:
            return VK_FORMAT_R16G16_SNORM;
        case GERIUM_FORMAT_R16G16_UINT:
            return VK_FORMAT_R16G16_UINT;
        case GERIUM_FORMAT_R16G16_SINT:
            return VK_FORMAT_R16G16_SINT;
        case GERIUM_FORMAT_R16G16_SFLOAT:
            return VK_FORMAT_R16G16_SFLOAT;
        case GERIUM_FORMAT_R16G16_USCALED:
            return VK_FORMAT_R16G16_USCALED;
        case GERIUM_FORMAT_R16G16_SSCALED:
            return VK_FORMAT_R16G16_SSCALED;
        case GERIUM_FORMAT_R16G16B16_UNORM:
            return VK_FORMAT_R16G16B16_UNORM;
        case GERIUM_FORMAT_R16G16B16_SNORM:
            return VK_FORMAT_R16G16B16_SNORM;
        case GERIUM_FORMAT_R16G16B16_UINT:
            return VK_FORMAT_R16G16B16_UINT;
        case GERIUM_FORMAT_R16G16B16_SINT:
            return VK_FORMAT_R16G16B16_SINT;
        case GERIUM_FORMAT_R16G16B16_SFLOAT:
            return VK_FORMAT_R16G16B16_SFLOAT;
        case GERIUM_FORMAT_R16G16B16_USCALED:
            return VK_FORMAT_R16G16B16_USCALED;
        case GERIUM_FORMAT_R16G16B16_SSCALED:
            return VK_FORMAT_R16G16B16_SSCALED;
        case GERIUM_FORMAT_R16G16B16A16_UNORM:
            return VK_FORMAT_R16G16B16A16_UNORM;
        case GERIUM_FORMAT_R16G16B16A16_SNORM:
            return VK_FORMAT_R16G16B16A16_SNORM;
        case GERIUM_FORMAT_R16G16B16A16_UINT:
            return VK_FORMAT_R16G16B16A16_UINT;
        case GERIUM_FORMAT_R16G16B16A16_SINT:
            return VK_FORMAT_R16G16B16A16_SINT;
        case GERIUM_FORMAT_R16G16B16A16_SFLOAT:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case GERIUM_FORMAT_R16G16B16A16_USCALED:
            return VK_FORMAT_R16G16B16A16_USCALED;
        case GERIUM_FORMAT_R16G16B16A16_SSCALED:
            return VK_FORMAT_R16G16B16A16_SSCALED;
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
        case GERIUM_FORMAT_R64_UINT:
            return VK_FORMAT_R64_UINT;
        case GERIUM_FORMAT_R64_SINT:
            return VK_FORMAT_R64_SINT;
        case GERIUM_FORMAT_R64_SFLOAT:
            return VK_FORMAT_R64_SFLOAT;
        case GERIUM_FORMAT_R64G64_UINT:
            return VK_FORMAT_R64G64_UINT;
        case GERIUM_FORMAT_R64G64_SINT:
            return VK_FORMAT_R64G64_SINT;
        case GERIUM_FORMAT_R64G64_SFLOAT:
            return VK_FORMAT_R64G64_SFLOAT;
        case GERIUM_FORMAT_R64G64B64_UINT:
            return VK_FORMAT_R64G64B64_UINT;
        case GERIUM_FORMAT_R64G64B64_SINT:
            return VK_FORMAT_R64G64B64_SINT;
        case GERIUM_FORMAT_R64G64B64_SFLOAT:
            return VK_FORMAT_R64G64B64_SFLOAT;
        case GERIUM_FORMAT_R64G64B64A64_UINT:
            return VK_FORMAT_R64G64B64A64_UINT;
        case GERIUM_FORMAT_R64G64B64A64_SINT:
            return VK_FORMAT_R64G64B64A64_SINT;
        case GERIUM_FORMAT_R64G64B64A64_SFLOAT:
            return VK_FORMAT_R64G64B64A64_SFLOAT;
        case GERIUM_FORMAT_B4G4R4A4_UNORM:
            return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
        case GERIUM_FORMAT_B5G5R5A1_UNORM:
            return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
        case GERIUM_FORMAT_B5G6R5_UNORM:
            return VK_FORMAT_B5G6R5_UNORM_PACK16;
        case GERIUM_FORMAT_B8G8R8_UNORM:
            return VK_FORMAT_B8G8R8_UNORM;
        case GERIUM_FORMAT_B8G8R8_SNORM:
            return VK_FORMAT_B8G8R8_SNORM;
        case GERIUM_FORMAT_B8G8R8_UINT:
            return VK_FORMAT_B8G8R8_UINT;
        case GERIUM_FORMAT_B8G8R8_SINT:
            return VK_FORMAT_B8G8R8_SINT;
        case GERIUM_FORMAT_B8G8R8_SRGB:
            return VK_FORMAT_B8G8R8_SRGB;
        case GERIUM_FORMAT_B8G8R8_USCALED:
            return VK_FORMAT_B8G8R8_USCALED;
        case GERIUM_FORMAT_B8G8R8_SSCALED:
            return VK_FORMAT_B8G8R8_SSCALED;
        case GERIUM_FORMAT_B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        case GERIUM_FORMAT_B8G8R8A8_SNORM:
            return VK_FORMAT_B8G8R8A8_SNORM;
        case GERIUM_FORMAT_B8G8R8A8_UINT:
            return VK_FORMAT_B8G8R8A8_UINT;
        case GERIUM_FORMAT_B8G8R8A8_SINT:
            return VK_FORMAT_B8G8R8A8_SINT;
        case GERIUM_FORMAT_B8G8R8A8_SRGB:
            return VK_FORMAT_B8G8R8A8_SRGB;
        case GERIUM_FORMAT_B8G8R8A8_USCALED:
            return VK_FORMAT_B8G8R8A8_USCALED;
        case GERIUM_FORMAT_B8G8R8A8_SSCALED:
            return VK_FORMAT_B8G8R8A8_SSCALED;
        case GERIUM_FORMAT_B10G11R11_UFLOAT:
            return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
        case GERIUM_FORMAT_A1B5G5R5_UNORM:
            return VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR;
        case GERIUM_FORMAT_A1R5G5B5_UNORM:
            return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
        case GERIUM_FORMAT_A2B10G10R10_UNORM:
            return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
        case GERIUM_FORMAT_A2B10G10R10_SNORM:
            return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
        case GERIUM_FORMAT_A2B10G10R10_UINT:
            return VK_FORMAT_A2B10G10R10_UINT_PACK32;
        case GERIUM_FORMAT_A2B10G10R10_SINT:
            return VK_FORMAT_A2B10G10R10_SINT_PACK32;
        case GERIUM_FORMAT_A2B10G10R10_USCALED:
            return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
        case GERIUM_FORMAT_A2B10G10R10_SSCALED:
            return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_UNORM:
            return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_SNORM:
            return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_UINT:
            return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_SINT:
            return VK_FORMAT_A2R10G10B10_SINT_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_USCALED:
            return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
        case GERIUM_FORMAT_A2R10G10B10_SSCALED:
            return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
        case GERIUM_FORMAT_A4B4G4R4_UNORM:
            return VK_FORMAT_A4B4G4R4_UNORM_PACK16;
        case GERIUM_FORMAT_A4R4G4B4_UNORM:
            return VK_FORMAT_A4R4G4B4_UNORM_PACK16;
        case GERIUM_FORMAT_A8_UNORM:
            return VK_FORMAT_A8_UNORM_KHR;
        case GERIUM_FORMAT_A8B8G8R8_UNORM:
            return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
        case GERIUM_FORMAT_A8B8G8R8_SNORM:
            return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
        case GERIUM_FORMAT_A8B8G8R8_UINT:
            return VK_FORMAT_A8B8G8R8_UINT_PACK32;
        case GERIUM_FORMAT_A8B8G8R8_SINT:
            return VK_FORMAT_A8B8G8R8_SINT_PACK32;
        case GERIUM_FORMAT_A8B8G8R8_SRGB:
            return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
        case GERIUM_FORMAT_A8B8G8R8_USCALED:
            return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
        case GERIUM_FORMAT_A8B8G8R8_SSCALED:
            return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
        case GERIUM_FORMAT_S8_UINT:
            return VK_FORMAT_S8_UINT;
        case GERIUM_FORMAT_X8_D24_UNORM:
            return VK_FORMAT_X8_D24_UNORM_PACK32;
        case GERIUM_FORMAT_D16_UNORM:
            return VK_FORMAT_D16_UNORM;
        case GERIUM_FORMAT_D16_UNORM_S8_UINT:
            return VK_FORMAT_D16_UNORM_S8_UINT;
        case GERIUM_FORMAT_D24_UNORM_S8_UINT:
            return VK_FORMAT_D24_UNORM_S8_UINT;
        case GERIUM_FORMAT_D32_SFLOAT:
            return VK_FORMAT_D32_SFLOAT;
        case GERIUM_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case GERIUM_FORMAT_ASTC_4x4_UNORM:
            return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_4x4_SRGB:
            return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_4x4_SFLOAT:
            return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_5x4_UNORM:
            return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_5x4_SRGB:
            return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_5x4_SFLOAT:
            return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_5x5_UNORM:
            return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_5x5_SRGB:
            return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_5x5_SFLOAT:
            return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_6x5_UNORM:
            return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_6x5_SRGB:
            return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_6x5_SFLOAT:
            return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_6x6_UNORM:
            return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_6x6_SRGB:
            return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_6x6_SFLOAT:
            return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_8x5_UNORM:
            return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_8x5_SRGB:
            return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_8x5_SFLOAT:
            return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_8x6_UNORM:
            return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_8x6_SRGB:
            return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_8x6_SFLOAT:
            return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_8x8_UNORM:
            return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_8x8_SRGB:
            return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_8x8_SFLOAT:
            return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_10x5_UNORM:
            return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_10x5_SRGB:
            return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_10x5_SFLOAT:
            return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_10x6_UNORM:
            return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_10x6_SRGB:
            return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_10x6_SFLOAT:
            return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_10x8_UNORM:
            return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_10x8_SRGB:
            return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_10x8_SFLOAT:
            return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_10x10_UNORM:
            return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_10x10_SRGB:
            return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_10x10_SFLOAT:
            return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_12x10_UNORM:
            return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_12x10_SRGB:
            return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_12x10_SFLOAT:
            return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK;
        case GERIUM_FORMAT_ASTC_12x12_UNORM:
            return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
        case GERIUM_FORMAT_ASTC_12x12_SRGB:
            return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
        case GERIUM_FORMAT_ASTC_12x12_SFLOAT:
            return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK;
        case GERIUM_FORMAT_BC1_RGB_UNORM:
            return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case GERIUM_FORMAT_BC1_RGB_SRGB:
            return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
        case GERIUM_FORMAT_BC1_RGBA_UNORM:
            return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case GERIUM_FORMAT_BC1_RGBA_SRGB:
            return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
        case GERIUM_FORMAT_BC2_UNORM:
            return VK_FORMAT_BC2_UNORM_BLOCK;
        case GERIUM_FORMAT_BC2_SRGB:
            return VK_FORMAT_BC2_SRGB_BLOCK;
        case GERIUM_FORMAT_BC3_UNORM:
            return VK_FORMAT_BC3_UNORM_BLOCK;
        case GERIUM_FORMAT_BC3_SRGB:
            return VK_FORMAT_BC3_SRGB_BLOCK;
        case GERIUM_FORMAT_BC4_UNORM:
            return VK_FORMAT_BC4_UNORM_BLOCK;
        case GERIUM_FORMAT_BC4_SNORM:
            return VK_FORMAT_BC4_SNORM_BLOCK;
        case GERIUM_FORMAT_BC5_UNORM:
            return VK_FORMAT_BC5_UNORM_BLOCK;
        case GERIUM_FORMAT_BC5_SNORM:
            return VK_FORMAT_BC5_SNORM_BLOCK;
        case GERIUM_FORMAT_BC6H_UFLOAT:
            return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case GERIUM_FORMAT_BC6H_SFLOAT:
            return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case GERIUM_FORMAT_BC7_UNORM:
            return VK_FORMAT_BC7_UNORM_BLOCK;
        case GERIUM_FORMAT_BC7_SRGB:
            return VK_FORMAT_BC7_SRGB_BLOCK;
        case GERIUM_FORMAT_ETC2_R8G8B8_UNORM:
            return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
        case GERIUM_FORMAT_ETC2_R8G8B8_SRGB:
            return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
        case GERIUM_FORMAT_ETC2_R8G8B8A1_UNORM:
            return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
        case GERIUM_FORMAT_ETC2_R8G8B8A1_SRGB:
            return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
        case GERIUM_FORMAT_ETC2_R8G8B8A8_UNORM:
            return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
        case GERIUM_FORMAT_ETC2_R8G8B8A8_SRGB:
            return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
        case GERIUM_FORMAT_EAC_R11_UNORM:
            return VK_FORMAT_EAC_R11_UNORM_BLOCK;
        case GERIUM_FORMAT_EAC_R11_SNORM:
            return VK_FORMAT_EAC_R11_SNORM_BLOCK;
        case GERIUM_FORMAT_EAC_R11G11_UNORM:
            return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
        case GERIUM_FORMAT_EAC_R11G11_SNORM:
            return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
        case GERIUM_FORMAT_PVRTC1_2BPP_UNORM:
            return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
        case GERIUM_FORMAT_PVRTC1_2BPP_SRGB:
            return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
        case GERIUM_FORMAT_PVRTC1_4BPP_UNORM:
            return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
        case GERIUM_FORMAT_PVRTC1_4BPP_SRGB:
            return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
        case GERIUM_FORMAT_PVRTC2_2BPP_UNORM:
            return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
        case GERIUM_FORMAT_PVRTC2_2BPP_SRGB:
            return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
        case GERIUM_FORMAT_PVRTC2_4BPP_UNORM:
            return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
        case GERIUM_FORMAT_PVRTC2_4BPP_SRGB:
            return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;
        default:
            assert(!"unreachable code");
            return VK_FORMAT_UNDEFINED;
    }
}

gerium_inline gerium_format_t toGeriumFormat(VkFormat format) noexcept {
    switch (format) {
        case VK_FORMAT_R4G4_UNORM_PACK8:
            return GERIUM_FORMAT_R4G4_UNORM;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
            return GERIUM_FORMAT_R4G4B4A4_UNORM;
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
            return GERIUM_FORMAT_R5G5B5A1_UNORM;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return GERIUM_FORMAT_R5G6B5_UNORM;
        case VK_FORMAT_R8_UNORM:
            return GERIUM_FORMAT_R8_UNORM;
        case VK_FORMAT_R8_SNORM:
            return GERIUM_FORMAT_R8_SNORM;
        case VK_FORMAT_R8_UINT:
            return GERIUM_FORMAT_R8_UINT;
        case VK_FORMAT_R8_SINT:
            return GERIUM_FORMAT_R8_SINT;
        case VK_FORMAT_R8_SRGB:
            return GERIUM_FORMAT_R8_SRGB;
        case VK_FORMAT_R8_USCALED:
            return GERIUM_FORMAT_R8_USCALED;
        case VK_FORMAT_R8_SSCALED:
            return GERIUM_FORMAT_R8_SSCALED;
        case VK_FORMAT_R8G8_UNORM:
            return GERIUM_FORMAT_R8G8_UNORM;
        case VK_FORMAT_R8G8_SNORM:
            return GERIUM_FORMAT_R8G8_SNORM;
        case VK_FORMAT_R8G8_UINT:
            return GERIUM_FORMAT_R8G8_UINT;
        case VK_FORMAT_R8G8_SINT:
            return GERIUM_FORMAT_R8G8_SINT;
        case VK_FORMAT_R8G8_SRGB:
            return GERIUM_FORMAT_R8G8_SRGB;
        case VK_FORMAT_R8G8_USCALED:
            return GERIUM_FORMAT_R8G8_USCALED;
        case VK_FORMAT_R8G8_SSCALED:
            return GERIUM_FORMAT_R8G8_SSCALED;
        case VK_FORMAT_R8G8B8_UNORM:
            return GERIUM_FORMAT_R8G8B8_UNORM;
        case VK_FORMAT_R8G8B8_SNORM:
            return GERIUM_FORMAT_R8G8B8_SNORM;
        case VK_FORMAT_R8G8B8_UINT:
            return GERIUM_FORMAT_R8G8B8_UINT;
        case VK_FORMAT_R8G8B8_SINT:
            return GERIUM_FORMAT_R8G8B8_SINT;
        case VK_FORMAT_R8G8B8_SRGB:
            return GERIUM_FORMAT_R8G8B8_SRGB;
        case VK_FORMAT_R8G8B8_USCALED:
            return GERIUM_FORMAT_R8G8B8_USCALED;
        case VK_FORMAT_R8G8B8_SSCALED:
            return GERIUM_FORMAT_R8G8B8_SSCALED;
        case VK_FORMAT_R8G8B8A8_UNORM:
            return GERIUM_FORMAT_R8G8B8A8_UNORM;
        case VK_FORMAT_R8G8B8A8_SNORM:
            return GERIUM_FORMAT_R8G8B8A8_SNORM;
        case VK_FORMAT_R8G8B8A8_UINT:
            return GERIUM_FORMAT_R8G8B8A8_UINT;
        case VK_FORMAT_R8G8B8A8_SINT:
            return GERIUM_FORMAT_R8G8B8A8_SINT;
        case VK_FORMAT_R8G8B8A8_SRGB:
            return GERIUM_FORMAT_R8G8B8A8_SRGB;
        case VK_FORMAT_R8G8B8A8_USCALED:
            return GERIUM_FORMAT_R8G8B8A8_USCALED;
        case VK_FORMAT_R8G8B8A8_SSCALED:
            return GERIUM_FORMAT_R8G8B8A8_SSCALED;
        case VK_FORMAT_R16_UNORM:
            return GERIUM_FORMAT_R16_UNORM;
        case VK_FORMAT_R16_SNORM:
            return GERIUM_FORMAT_R16_SNORM;
        case VK_FORMAT_R16_UINT:
            return GERIUM_FORMAT_R16_UINT;
        case VK_FORMAT_R16_SINT:
            return GERIUM_FORMAT_R16_SINT;
        case VK_FORMAT_R16_SFLOAT:
            return GERIUM_FORMAT_R16_SFLOAT;
        case VK_FORMAT_R16_USCALED:
            return GERIUM_FORMAT_R16_USCALED;
        case VK_FORMAT_R16_SSCALED:
            return GERIUM_FORMAT_R16_SSCALED;
        case VK_FORMAT_R16G16_UNORM:
            return GERIUM_FORMAT_R16G16_UNORM;
        case VK_FORMAT_R16G16_SNORM:
            return GERIUM_FORMAT_R16G16_SNORM;
        case VK_FORMAT_R16G16_UINT:
            return GERIUM_FORMAT_R16G16_UINT;
        case VK_FORMAT_R16G16_SINT:
            return GERIUM_FORMAT_R16G16_SINT;
        case VK_FORMAT_R16G16_SFLOAT:
            return GERIUM_FORMAT_R16G16_SFLOAT;
        case VK_FORMAT_R16G16_USCALED:
            return GERIUM_FORMAT_R16G16_USCALED;
        case VK_FORMAT_R16G16_SSCALED:
            return GERIUM_FORMAT_R16G16_SSCALED;
        case VK_FORMAT_R16G16B16_UNORM:
            return GERIUM_FORMAT_R16G16B16_UNORM;
        case VK_FORMAT_R16G16B16_SNORM:
            return GERIUM_FORMAT_R16G16B16_SNORM;
        case VK_FORMAT_R16G16B16_UINT:
            return GERIUM_FORMAT_R16G16B16_UINT;
        case VK_FORMAT_R16G16B16_SINT:
            return GERIUM_FORMAT_R16G16B16_SINT;
        case VK_FORMAT_R16G16B16_SFLOAT:
            return GERIUM_FORMAT_R16G16B16_SFLOAT;
        case VK_FORMAT_R16G16B16_USCALED:
            return GERIUM_FORMAT_R16G16B16_USCALED;
        case VK_FORMAT_R16G16B16_SSCALED:
            return GERIUM_FORMAT_R16G16B16_SSCALED;
        case VK_FORMAT_R16G16B16A16_UNORM:
            return GERIUM_FORMAT_R16G16B16A16_UNORM;
        case VK_FORMAT_R16G16B16A16_SNORM:
            return GERIUM_FORMAT_R16G16B16A16_SNORM;
        case VK_FORMAT_R16G16B16A16_UINT:
            return GERIUM_FORMAT_R16G16B16A16_UINT;
        case VK_FORMAT_R16G16B16A16_SINT:
            return GERIUM_FORMAT_R16G16B16A16_SINT;
        case VK_FORMAT_R16G16B16A16_SFLOAT:
            return GERIUM_FORMAT_R16G16B16A16_SFLOAT;
        case VK_FORMAT_R16G16B16A16_USCALED:
            return GERIUM_FORMAT_R16G16B16A16_USCALED;
        case VK_FORMAT_R16G16B16A16_SSCALED:
            return GERIUM_FORMAT_R16G16B16A16_SSCALED;
        case VK_FORMAT_R32_UINT:
            return GERIUM_FORMAT_R32_UINT;
        case VK_FORMAT_R32_SINT:
            return GERIUM_FORMAT_R32_SINT;
        case VK_FORMAT_R32_SFLOAT:
            return GERIUM_FORMAT_R32_SFLOAT;
        case VK_FORMAT_R32G32_UINT:
            return GERIUM_FORMAT_R32G32_UINT;
        case VK_FORMAT_R32G32_SINT:
            return GERIUM_FORMAT_R32G32_SINT;
        case VK_FORMAT_R32G32_SFLOAT:
            return GERIUM_FORMAT_R32G32_SFLOAT;
        case VK_FORMAT_R32G32B32_UINT:
            return GERIUM_FORMAT_R32G32B32_UINT;
        case VK_FORMAT_R32G32B32_SINT:
            return GERIUM_FORMAT_R32G32B32_SINT;
        case VK_FORMAT_R32G32B32_SFLOAT:
            return GERIUM_FORMAT_R32G32B32_SFLOAT;
        case VK_FORMAT_R32G32B32A32_UINT:
            return GERIUM_FORMAT_R32G32B32A32_UINT;
        case VK_FORMAT_R32G32B32A32_SINT:
            return GERIUM_FORMAT_R32G32B32A32_SINT;
        case VK_FORMAT_R32G32B32A32_SFLOAT:
            return GERIUM_FORMAT_R32G32B32A32_SFLOAT;
        case VK_FORMAT_R64_UINT:
            return GERIUM_FORMAT_R64_UINT;
        case VK_FORMAT_R64_SINT:
            return GERIUM_FORMAT_R64_SINT;
        case VK_FORMAT_R64_SFLOAT:
            return GERIUM_FORMAT_R64_SFLOAT;
        case VK_FORMAT_R64G64_UINT:
            return GERIUM_FORMAT_R64G64_UINT;
        case VK_FORMAT_R64G64_SINT:
            return GERIUM_FORMAT_R64G64_SINT;
        case VK_FORMAT_R64G64_SFLOAT:
            return GERIUM_FORMAT_R64G64_SFLOAT;
        case VK_FORMAT_R64G64B64_UINT:
            return GERIUM_FORMAT_R64G64B64_UINT;
        case VK_FORMAT_R64G64B64_SINT:
            return GERIUM_FORMAT_R64G64B64_SINT;
        case VK_FORMAT_R64G64B64_SFLOAT:
            return GERIUM_FORMAT_R64G64B64_SFLOAT;
        case VK_FORMAT_R64G64B64A64_UINT:
            return GERIUM_FORMAT_R64G64B64A64_UINT;
        case VK_FORMAT_R64G64B64A64_SINT:
            return GERIUM_FORMAT_R64G64B64A64_SINT;
        case VK_FORMAT_R64G64B64A64_SFLOAT:
            return GERIUM_FORMAT_R64G64B64A64_SFLOAT;
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
            return GERIUM_FORMAT_B4G4R4A4_UNORM;
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
            return GERIUM_FORMAT_B5G5R5A1_UNORM;
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
            return GERIUM_FORMAT_B5G6R5_UNORM;
        case VK_FORMAT_B8G8R8_UNORM:
            return GERIUM_FORMAT_B8G8R8_UNORM;
        case VK_FORMAT_B8G8R8_SNORM:
            return GERIUM_FORMAT_B8G8R8_SNORM;
        case VK_FORMAT_B8G8R8_UINT:
            return GERIUM_FORMAT_B8G8R8_UINT;
        case VK_FORMAT_B8G8R8_SINT:
            return GERIUM_FORMAT_B8G8R8_SINT;
        case VK_FORMAT_B8G8R8_SRGB:
            return GERIUM_FORMAT_B8G8R8_SRGB;
        case VK_FORMAT_B8G8R8_USCALED:
            return GERIUM_FORMAT_B8G8R8_USCALED;
        case VK_FORMAT_B8G8R8_SSCALED:
            return GERIUM_FORMAT_B8G8R8_SSCALED;
        case VK_FORMAT_B8G8R8A8_UNORM:
            return GERIUM_FORMAT_B8G8R8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_SNORM:
            return GERIUM_FORMAT_B8G8R8A8_SNORM;
        case VK_FORMAT_B8G8R8A8_UINT:
            return GERIUM_FORMAT_B8G8R8A8_UINT;
        case VK_FORMAT_B8G8R8A8_SINT:
            return GERIUM_FORMAT_B8G8R8A8_SINT;
        case VK_FORMAT_B8G8R8A8_SRGB:
            return GERIUM_FORMAT_B8G8R8A8_SRGB;
        case VK_FORMAT_B8G8R8A8_USCALED:
            return GERIUM_FORMAT_B8G8R8A8_USCALED;
        case VK_FORMAT_B8G8R8A8_SSCALED:
            return GERIUM_FORMAT_B8G8R8A8_SSCALED;
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
            return GERIUM_FORMAT_B10G11R11_UFLOAT;
        case VK_FORMAT_A1B5G5R5_UNORM_PACK16_KHR:
            return GERIUM_FORMAT_A1B5G5R5_UNORM;
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
            return GERIUM_FORMAT_A1R5G5B5_UNORM;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return GERIUM_FORMAT_A2B10G10R10_UNORM;
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
            return GERIUM_FORMAT_A2B10G10R10_SNORM;
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
            return GERIUM_FORMAT_A2B10G10R10_UINT;
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
            return GERIUM_FORMAT_A2B10G10R10_SINT;
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
            return GERIUM_FORMAT_A2B10G10R10_USCALED;
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
            return GERIUM_FORMAT_A2B10G10R10_SSCALED;
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
            return GERIUM_FORMAT_A2R10G10B10_UNORM;
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
            return GERIUM_FORMAT_A2R10G10B10_SNORM;
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            return GERIUM_FORMAT_A2R10G10B10_UINT;
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
            return GERIUM_FORMAT_A2R10G10B10_SINT;
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
            return GERIUM_FORMAT_A2R10G10B10_USCALED;
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
            return GERIUM_FORMAT_A2R10G10B10_SSCALED;
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
            return GERIUM_FORMAT_A4B4G4R4_UNORM;
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
            return GERIUM_FORMAT_A4R4G4B4_UNORM;
        case VK_FORMAT_A8_UNORM_KHR:
            return GERIUM_FORMAT_A8_UNORM;
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
            return GERIUM_FORMAT_A8B8G8R8_UNORM;
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
            return GERIUM_FORMAT_A8B8G8R8_SNORM;
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
            return GERIUM_FORMAT_A8B8G8R8_UINT;
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
            return GERIUM_FORMAT_A8B8G8R8_SINT;
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
            return GERIUM_FORMAT_A8B8G8R8_SRGB;
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
            return GERIUM_FORMAT_A8B8G8R8_USCALED;
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
            return GERIUM_FORMAT_A8B8G8R8_SSCALED;
        case VK_FORMAT_S8_UINT:
            return GERIUM_FORMAT_S8_UINT;
        case VK_FORMAT_X8_D24_UNORM_PACK32:
            return GERIUM_FORMAT_X8_D24_UNORM;
        case VK_FORMAT_D16_UNORM:
            return GERIUM_FORMAT_D16_UNORM;
        case VK_FORMAT_D16_UNORM_S8_UINT:
            return GERIUM_FORMAT_D16_UNORM_S8_UINT;
        case VK_FORMAT_D24_UNORM_S8_UINT:
            return GERIUM_FORMAT_D24_UNORM_S8_UINT;
        case VK_FORMAT_D32_SFLOAT:
            return GERIUM_FORMAT_D32_SFLOAT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return GERIUM_FORMAT_D32_SFLOAT_S8_UINT;
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_4x4_UNORM;
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_4x4_SRGB;
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_4x4_SFLOAT;
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_5x4_UNORM;
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_5x4_SRGB;
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_5x4_SFLOAT;
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_5x5_UNORM;
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_5x5_SRGB;
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_5x5_SFLOAT;
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_6x5_UNORM;
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_6x5_SRGB;
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_6x5_SFLOAT;
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_6x6_UNORM;
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_6x6_SRGB;
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_6x6_SFLOAT;
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_8x5_UNORM;
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_8x5_SRGB;
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_8x5_SFLOAT;
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_8x6_UNORM;
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_8x6_SRGB;
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_8x6_SFLOAT;
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_8x8_UNORM;
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_8x8_SRGB;
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_8x8_SFLOAT;
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_10x5_UNORM;
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_10x5_SRGB;
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_10x5_SFLOAT;
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_10x6_UNORM;
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_10x6_SRGB;
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_10x6_SFLOAT;
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_10x8_UNORM;
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_10x8_SRGB;
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_10x8_SFLOAT;
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_10x10_UNORM;
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_10x10_SRGB;
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_10x10_SFLOAT;
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_12x10_UNORM;
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_12x10_SRGB;
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_12x10_SFLOAT;
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
            return GERIUM_FORMAT_ASTC_12x12_UNORM;
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
            return GERIUM_FORMAT_ASTC_12x12_SRGB;
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
            return GERIUM_FORMAT_ASTC_12x12_SFLOAT;
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
            return GERIUM_FORMAT_BC1_RGB_UNORM;
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
            return GERIUM_FORMAT_BC1_RGB_SRGB;
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
            return GERIUM_FORMAT_BC1_RGBA_UNORM;
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
            return GERIUM_FORMAT_BC1_RGBA_SRGB;
        case VK_FORMAT_BC2_UNORM_BLOCK:
            return GERIUM_FORMAT_BC2_UNORM;
        case VK_FORMAT_BC2_SRGB_BLOCK:
            return GERIUM_FORMAT_BC2_SRGB;
        case VK_FORMAT_BC3_UNORM_BLOCK:
            return GERIUM_FORMAT_BC3_UNORM;
        case VK_FORMAT_BC3_SRGB_BLOCK:
            return GERIUM_FORMAT_BC3_SRGB;
        case VK_FORMAT_BC4_UNORM_BLOCK:
            return GERIUM_FORMAT_BC4_UNORM;
        case VK_FORMAT_BC4_SNORM_BLOCK:
            return GERIUM_FORMAT_BC4_SNORM;
        case VK_FORMAT_BC5_UNORM_BLOCK:
            return GERIUM_FORMAT_BC5_UNORM;
        case VK_FORMAT_BC5_SNORM_BLOCK:
            return GERIUM_FORMAT_BC5_SNORM;
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
            return GERIUM_FORMAT_BC6H_UFLOAT;
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
            return GERIUM_FORMAT_BC6H_SFLOAT;
        case VK_FORMAT_BC7_UNORM_BLOCK:
            return GERIUM_FORMAT_BC7_UNORM;
        case VK_FORMAT_BC7_SRGB_BLOCK:
            return GERIUM_FORMAT_BC7_SRGB;
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
            return GERIUM_FORMAT_ETC2_R8G8B8_UNORM;
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
            return GERIUM_FORMAT_ETC2_R8G8B8_SRGB;
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
            return GERIUM_FORMAT_ETC2_R8G8B8A1_UNORM;
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
            return GERIUM_FORMAT_ETC2_R8G8B8A1_SRGB;
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
            return GERIUM_FORMAT_ETC2_R8G8B8A8_UNORM;
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
            return GERIUM_FORMAT_ETC2_R8G8B8A8_SRGB;
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
            return GERIUM_FORMAT_EAC_R11_UNORM;
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
            return GERIUM_FORMAT_EAC_R11_SNORM;
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
            return GERIUM_FORMAT_EAC_R11G11_UNORM;
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
            return GERIUM_FORMAT_EAC_R11G11_SNORM;
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC1_2BPP_UNORM;
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC1_2BPP_SRGB;
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC1_4BPP_UNORM;
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC1_4BPP_SRGB;
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC2_2BPP_UNORM;
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC2_2BPP_SRGB;
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC2_4BPP_UNORM;
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
            return GERIUM_FORMAT_PVRTC2_4BPP_SRGB;
        default:
            assert(!"unreachable code");
            return {};
    }
}

template <typename T>
gerium_inline bool contains(const std::vector<T>& v, T item) noexcept {
    return std::find(v.cbegin(), v.cend(), item) != v.cend();
}

gerium_inline bool contains(const std::vector<const char*>& v, const char* item) noexcept {
    return std::find_if(v.cbegin(), v.cend(), [item](const auto value) {
        return value == item || strcmp(value, item) == 0;
    }) != v.cend();
}

gerium_inline gerium_uint32_t calcTextureSize(gerium_uint16_t width,
                                              gerium_uint16_t height,
                                              gerium_uint16_t depth,
                                              gerium_format_t format) noexcept {
    return width * height * depth * formatBlockSize(format);
}

gerium_uint32_t nameColor(gerium_utf8_t name);

glm::vec4 nameColorVec4(gerium_utf8_t name);

} // namespace gerium

#endif
