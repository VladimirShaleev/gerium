#ifndef GERIUM_GERIUM_HPP
#define GERIUM_GERIUM_HPP

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

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

} // namespace gerium

#endif
