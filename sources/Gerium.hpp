#ifndef GERIUM_GERIUM_HPP
#define GERIUM_GERIUM_HPP

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <utf8h/utf8.h>

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
