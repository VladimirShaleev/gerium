/**
 * @file      gerium-platform.h
 * @brief     Platform-specific definitions and utilities.
 * @details   This header provides cross-platform macros, type definitions, and utility
 *            macros for the Gerium library. It handles:
 *            - Platform detection (Windows, macOS, iOS, Android, Linux, Web)
 *            - Symbol visibility control (DLL import/export on Windows)
 *            - C/C++ interoperability
 *            - Type definitions for consistent data sizes across platforms
 *            - Bit flag operations for enumerations (C++ only).
 *            
 * @author    Vladimir Shaleev <vladimirshaleev@gmail.com>
 * @copyright MIT License
 */
#ifndef GERIUM_PLATFORM_H
#define GERIUM_PLATFORM_H

/**
 * @def     GERIUM_BEGIN
 * @brief   Begins a C-linkage declaration block.
 * @details In C++, expands to `extern "C" {` to ensure C-compatible symbol naming.
 *          In pure C environments, expands to nothing.
 * @sa      GERIUM_END
 *
 */

/**
 * @def     GERIUM_END
 * @brief   Ends a C-linkage declaration block.
 * @details Closes the scope opened by #GERIUM_BEGIN.
 * @sa      GERIUM_BEGIN
 *
 */

#ifdef __cplusplus
# define GERIUM_BEGIN extern "C" {
# define GERIUM_END   }
#else
# define GERIUM_BEGIN
# define GERIUM_END
#endif

/**
 * @def     gerium_api
 * @brief   Controls symbol visibility for shared library builds.
 * @details This macro is used to control symbol visibility when building or using the library.
 *          On Windows (**MSVC**) with dynamic linking (non-static build), it expands to `__declspec(dllimport)`.
 *          In all other cases (static builds or non-Windows platforms), it expands to nothing.
 *          This allows proper importing of symbols from DLLs on Windows platforms.
 * @note    Define `GERIUM_STATIC_BUILD` for static library configuration.
 */

#ifndef gerium_api
# if defined(_MSC_VER) && !defined(GERIUM_STATIC_BUILD)
#  define gerium_api __declspec(dllimport)
# else
#  define gerium_api
# endif
#endif

#if defined(_WIN32) && !defined(GERIUM_PLATFORM_WINDOWS)
# define GERIUM_PLATFORM_WINDOWS
#elif defined(__APPLE__)
# include <TargetConditionals.h>
# include <unistd.h>
# if TARGET_OS_IPHONE && !defined(GERIUM_PLATFORM_IOS)
#  define GERIUM_PLATFORM_IOS
# elif TARGET_IPHONE_SIMULATOR && !defined(GERIUM_PLATFORM_IOS)
#  define GERIUM_PLATFORM_IOS
# elif TARGET_OS_MAC && !defined(GERIUM_PLATFORM_MAC_OS)
#  define GERIUM_PLATFORM_MAC_OS
# else
#  error unsupported Apple platform
# endif
#elif defined(__ANDROID__) && !defined(GERIUM_PLATFORM_ANDROID)
# define GERIUM_PLATFORM_ANDROID
#elif defined(__linux__) && !defined(GERIUM_PLATFORM_LINUX)
# define GERIUM_PLATFORM_LINUX
#elif defined(__EMSCRIPTEN__) && !defined(GERIUM_PLATFORM_WEB)
# define GERIUM_PLATFORM_WEB
#else
# error unsupported platform
#endif

#ifdef __cpp_constexpr
#  define GERIUM_CONSTEXPR constexpr
#  if __cpp_constexpr >= 201304L
#    define GERIUM_CONSTEXPR_14 constexpr
#  else
#    define GERIUM_CONSTEXPR_14
#  endif
#else
#  define GERIUM_CONSTEXPR
#  define GERIUM_CONSTEXPR_14
#endif

/**
 * @name  Platform-independent type definitions
 * @brief Fixed-size types guaranteed to work across all supported platforms
 * @{
 */
#include <stdint.h>
typedef char        gerium_char_t;    /**< symbol type */
typedef int32_t     gerium_bool_t;    /**< boolean type */
typedef int8_t      gerium_sint8_t;   /**< 8 bit signed integer */
typedef uint8_t     gerium_uint8_t;   /**< 8 bit unsigned integer */
typedef int16_t     gerium_sint16_t;  /**< 16 bit signed integer */
typedef uint16_t    gerium_uint16_t;  /**< 16 bit unsigned integer */
typedef int32_t     gerium_sint32_t;  /**< 32 bit signed integer */
typedef uint32_t    gerium_uint32_t;  /**< 32 bit unsigned integer */
typedef int64_t     gerium_sint64_t;  /**< 64 bit signed integer */
typedef uint64_t    gerium_uint64_t;  /**< 64 bit unsigned integer */
typedef float       gerium_float32_t; /**< 32 bit float point */
typedef double      gerium_float64_t; /**< 64 bit float point */
typedef const char* gerium_utf8_t;    /**< utf8 string */
typedef void*       gerium_data_t;    /**< pointer to data */
typedef const void* gerium_cdata_t;   /**< pointer to immutable data */
/** @} */

/**
 * @def       GERIUM_FLAGS
 * @brief     Enables bit flag operations for enumerations (C++ only).
 * @details   Generates overloaded bitwise operators for type-safe flag manipulation:
 *            - Bitwise NOT (~)
 *            - OR (|, |=)
 *            - AND (&, &=)
 *            - XOR (^, ^=)
 * 
 * @param[in] gerium_enum_t Enumeration type to enhance with flag operations
 * @note      Only active in C++ mode. In C, expands to nothing.
 */

#ifdef __cplusplus
# define GERIUM_FLAGS(gerium_enum_t) \
extern "C++" { \
inline GERIUM_CONSTEXPR gerium_enum_t operator~(gerium_enum_t lhr) noexcept { \
    return static_cast<gerium_enum_t>(~static_cast<gerium_sint32_t>(lhr)); \
} \
inline GERIUM_CONSTEXPR gerium_enum_t operator|(gerium_enum_t lhr, gerium_enum_t rhs) noexcept { \
    return static_cast<gerium_enum_t>(static_cast<gerium_sint32_t>(lhr) | static_cast<gerium_sint32_t>(rhs)); \
} \
inline GERIUM_CONSTEXPR gerium_enum_t operator&(gerium_enum_t lhr, gerium_enum_t rhs) noexcept { \
    return static_cast<gerium_enum_t>(static_cast<gerium_sint32_t>(lhr) & static_cast<gerium_sint32_t>(rhs)); \
} \
inline GERIUM_CONSTEXPR gerium_enum_t operator^(gerium_enum_t lhr, gerium_enum_t rhs) noexcept { \
    return static_cast<gerium_enum_t>(static_cast<gerium_sint32_t>(lhr) ^ static_cast<gerium_sint32_t>(rhs)); \
} \
inline GERIUM_CONSTEXPR_14 gerium_enum_t& operator|=(gerium_enum_t& lhr, gerium_enum_t rhs) noexcept { \
    return lhr = lhr | rhs; \
} \
inline GERIUM_CONSTEXPR_14 gerium_enum_t& operator&=(gerium_enum_t& lhr, gerium_enum_t rhs) noexcept { \
    return lhr = lhr & rhs; \
} \
inline GERIUM_CONSTEXPR_14 gerium_enum_t& operator^=(gerium_enum_t& lhr, gerium_enum_t rhs) noexcept { \
    return lhr = lhr ^ rhs; \
} \
}
#else
# define GERIUM_FLAGS(gerium_enum_t)
#endif

/**
 * @def       GERIUM_TYPE
 * @brief     Declares an opaque handle type.
 * @details   Creates a typedef for a pointer to an incomplete struct type,
 *            providing type safety while hiding implementation details.
 * @param[in] gerium_name Base name for the type (suffix `_t` will be added)
 */
#define GERIUM_TYPE(gerium_name) \
typedef struct _##gerium_name* gerium_name##_t;

/**
 * @def       GERIUM_HANDLE
 * @brief     Declares an index-based handle type.
 * @details   Creates a struct containing an index value, typically used for
 *            resource handles in API designs that avoid direct pointers.
 * @param[in] gerium_name Base name for the handle type (suffix `_h` will be added)
 */
#define GERIUM_HANDLE(gerium_name) \
typedef struct _##gerium_name { \
    gerium_uint16_t index; \
} gerium_name##_h;

#endif /* GERIUM_PLATFORM_H */
