#ifndef GERIUM_HANDLES_HPP
#define GERIUM_HANDLES_HPP

#include "Gerium.hpp"

namespace gerium {

struct Handle {
    gerium_uint16_t index;

    bool operator==(const Handle& rhs) const noexcept {
        return index == rhs.index;
    }

    bool operator!=(const Handle& rhs) const noexcept {
        return !(*this == rhs);
    }

    auto operator<=>(const Handle&) const = default;

    template <typename H>
    operator H() const noexcept {
        static_assert(sizeof(H) == sizeof(Handle));
        H result;
        memcpy(&result, &index, sizeof(H));
        return result;
    }
};

struct TextureHandle : Handle {};

} // namespace gerium

#endif
