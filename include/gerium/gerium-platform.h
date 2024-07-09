/**
 * \file      gerium-platform.h
 * \brief     define \a version of library
 * \author    Vladimir Shaleev
 * \copyright MIT License
 */

#ifndef GERIUM_PLATFORM_H
#define GERIUM_PLATFORM_H

#ifdef __cplusplus
# define GERIUM_BEGIN extern "C" {
# define GERIUM_END   }
#else
# define GERIUM_BEGIN
# define GERIUM_END
#endif

#ifndef gerium_public
# if defined(_MSC_VER) && !defined(GERIUM_STATIC_BUILD)
#  define gerium_public __declspec(dllimport)
# else
#  define gerium_public
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
#  error unsupported platform
# endif
#elif defined(__ANDROID__) && !defined(GERIUM_PLATFORM_ANDROID)
# define GERIUM_PLATFORM_ANDROID
#else
# error unsupported platform
#endif

#ifdef GERIUM_NO_STDINT_H
# if defined(_MSC_VER)
    typedef signed   __int8  int8_t;
    typedef unsigned __int8  uint8_t;
    typedef signed   __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef signed   __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef signed   __int64 int64_t;
    typedef unsigned __int64 uint64_t;
# else
#  error stdint types required
# endif
#else
# include <stdint.h>
#endif

typedef int32_t     gerium_bool_t;   
typedef int8_t      gerium_sint8_t;  
typedef uint8_t     gerium_uint8_t;  
typedef int16_t     gerium_sint16_t; 
typedef uint16_t    gerium_uint16_t; 
typedef int32_t     gerium_sint32_t; 
typedef uint32_t    gerium_uint32_t; 
typedef int64_t     gerium_sint64_t; 
typedef uint64_t    gerium_uint64_t; 
typedef float       gerium_float32_t;
typedef double      gerium_float64_t;
typedef const char* gerium_utf8_t;   
typedef void*       gerium_data_t;   
typedef const void* gerium_cdata_t;  

#ifdef __cplusplus
# define GERIUM_FLAGS(gerium_enum_t)                                                                          \
extern "C++" {                                                                                                \
inline gerium_enum_t operator|(gerium_enum_t lhr, gerium_enum_t rhs) noexcept {                               \
    return static_cast<gerium_enum_t>(static_cast<gerium_sint32_t>(lhr) | static_cast<gerium_sint32_t>(rhs)); \
}                                                                                                             \
inline gerium_enum_t operator&(gerium_enum_t lhr, gerium_enum_t rhs) noexcept {                               \
    return static_cast<gerium_enum_t>(static_cast<gerium_sint32_t>(lhr) & static_cast<gerium_sint32_t>(rhs)); \
}                                                                                                             \
inline gerium_enum_t operator^(gerium_enum_t lhr, gerium_enum_t rhs) noexcept {                               \
    return static_cast<gerium_enum_t>(static_cast<gerium_sint32_t>(lhr) ^ static_cast<gerium_sint32_t>(rhs)); \
}                                                                                                             \
inline gerium_enum_t& operator|=(gerium_enum_t& lhr, gerium_enum_t rhs) noexcept {                            \
    return lhr = lhr | rhs;                                                                                   \
}                                                                                                             \
inline gerium_enum_t& operator&=(gerium_enum_t& lhr, gerium_enum_t rhs) noexcept {                            \
    return lhr = lhr & rhs;                                                                                   \
}                                                                                                             \
inline gerium_enum_t& operator^=(gerium_enum_t& lhr, gerium_enum_t rhs) noexcept {                            \
    return lhr = lhr ^ rhs;                                                                                   \
}                                                                                                             \
}
#else
# define GERIUM_FLAGS(gerium_enum_t)
#endif

#define GERIUM_HANDLE(gerium_name) \
typedef struct {                   \
    gerium_uint16_t unused;        \
} gerium_name;

#endif
