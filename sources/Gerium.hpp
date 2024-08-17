#ifndef GERIUM_GERIUM_HPP
#define GERIUM_GERIUM_HPP

#define NOMINMAX
#define UNICODE
#define _UNICODE

#include "gerium/gerium-platform.h"
#include <locale>
#include <streambuf>
#ifndef GERIUM_PLATFORM_ANDROID
# include <mimalloc-override.h>

namespace std { // add mi_* functions to std for vulkan headers

inline mi_decl_restrict void* mi_malloc(size_t size) mi_attr_noexcept mi_attr_malloc mi_attr_alloc_size(1) {
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
# error unsupported platform
#elif defined(GERIUM_PLATFORM_MAC_OS)
# define VK_USE_PLATFORM_MACOS_MVK
# define VK_USE_PLATFORM_METAL_EXT
#elif defined(GERIUM_PLATFORM_ANDROID)
# define VK_USE_PLATFORM_ANDROID_KHR
#else
# error unsupported platform
#endif
#ifndef __APPLE__
# define VK_NO_PROTOTYPES
# define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 1
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

} // namespace gerium

#endif
