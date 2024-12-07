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
#include <wyhash.h>

// Vulkan
#if defined(GERIUM_PLATFORM_WINDOWS)
# define VK_USE_PLATFORM_WIN32_KHR
#elif defined(GERIUM_PLATFORM_IOS)
# define VK_USE_PLATFORM_METAL_EXT
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

// Stb
#include <stb_image.h>

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

gerium_inline gerium_uint64_t hash(std::string_view str, gerium_uint64_t seed = 0) noexcept {
    return wyhash(str.data(), str.length(), seed, _wyp);
}

gerium_inline gerium_uint64_t hash(gerium_utf8_t str, gerium_uint64_t seed = 0) noexcept {
    return wyhash(str, strlen(str), seed, _wyp);
}

template <typename T>
gerium_inline gerium_uint64_t hash(const T& data, gerium_uint64_t seed = 0) noexcept {
    return wyhash(&data, sizeof(T), seed, _wyp);
}

gerium_inline gerium_uint64_t hash(gerium_cdata_t data, gerium_uint32_t size, gerium_uint64_t seed = 0) noexcept {
    return wyhash(data, size, seed, _wyp);
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
        case GERIUM_FORMAT_R8_UNORM:
            return 1;
        case GERIUM_FORMAT_R8_SNORM:
            return 1;
        case GERIUM_FORMAT_R8_UINT:
            return 1;
        case GERIUM_FORMAT_R8_SINT:
            return 1;
        case GERIUM_FORMAT_R8G8_UNORM:
            return 2;
        case GERIUM_FORMAT_R8G8_SNORM:
            return 2;
        case GERIUM_FORMAT_R8G8_UINT:
            return 2;
        case GERIUM_FORMAT_R8G8_SINT:
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
        case GERIUM_FORMAT_R4G4B4A4_UNORM:
            return 2;
        case GERIUM_FORMAT_R5G5B5A1_UNORM:
            return 2;
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
        case GERIUM_FORMAT_A2R10G10B10_UNORM:
            return 4;
        case GERIUM_FORMAT_A2R10G10B10_UINT:
            return 4;
        case GERIUM_FORMAT_R16_UINT:
            return 2;
        case GERIUM_FORMAT_R16_SINT:
            return 2;
        case GERIUM_FORMAT_R16_SFLOAT:
            return 2;
        case GERIUM_FORMAT_R16G16_UINT:
            return 4;
        case GERIUM_FORMAT_R16G16_SINT:
            return 4;
        case GERIUM_FORMAT_R16G16_SFLOAT:
            return 4;
        case GERIUM_FORMAT_R16G16B16_UINT:
            return 6;
        case GERIUM_FORMAT_R16G16B16_SINT:
            return 6;
        case GERIUM_FORMAT_R16G16B16_SFLOAT:
            return 6;
        case GERIUM_FORMAT_R16G16B16A16_UINT:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_SINT:
            return 8;
        case GERIUM_FORMAT_R16G16B16A16_SFLOAT:
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
        case GERIUM_FORMAT_B10G11R11_UFLOAT:
            return 4;
        case GERIUM_FORMAT_E5B9G9R9_UFLOAT:
            return 4;
        case GERIUM_FORMAT_D16_UNORM:
            return 2;
        case GERIUM_FORMAT_X8_D24_UNORM:
            return 4;
        case GERIUM_FORMAT_D32_SFLOAT:
            return 4;
        case GERIUM_FORMAT_S8_UINT:
            return 1;
        case GERIUM_FORMAT_D24_UNORM_S8_UINT:
            return 4;
        case GERIUM_FORMAT_D32_SFLOAT_S8_UINT:
            return 5;
        default:
            assert(!"unreachable code");
            return 0;
    }
}

gerium_inline gerium_uint32_t calcTextureSize(gerium_uint16_t width,
                                              gerium_uint16_t height,
                                              gerium_uint16_t depth,
                                              gerium_format_t format) noexcept {
    return width * height * depth * formatBlockSize(format);
}

} // namespace gerium

#endif
