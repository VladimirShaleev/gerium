#ifndef GERIUM_GERIUM_HPP
#define GERIUM_GERIUM_HPP

#define NOMINMAX
#define UNICODE
#define _UNICODE

#include <mimalloc.h>

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

// Vulkan API
#if defined(_WIN32)
# define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__)
# include <TargetConditionals.h>
# include <unistd.h>
# if TARGET_OS_IPHONE
#  error unsupported platform
# elif TARGET_IPHONE_SIMULATOR
#  error unsupported platform
# elif TARGET_OS_MAC
#  define VK_USE_PLATFORM_MACOS_MVK
#  define VK_USE_PLATFORM_METAL_EXT
# else
#  error unsupported platform
# endif
#else
# error unsupported platform
#endif
#ifndef __APPLE__
# define VK_NO_PROTOTYPES
# define VULKAN_HPP_ENABLE_DYNAMIC_LOADER_TOOL 1
#endif
#include <vulkan/vulkan.hpp>

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

} // namespace gerium

#endif
