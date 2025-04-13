#ifndef COMPONENTS_CHANGES_HPP
#define COMPONENTS_CHANGES_HPP

#include "../Common.hpp"

enum class Change {
    None    = 0,
    Static  = 1,
    Dynamic = 2,
    All     = 3
};
GERIUM_FLAGS(Change);

struct Changes {
    Change transforms{};
    Change rigidBodies{};
    Change vehicles{};
    Change colliders{};
    Change renderables{};
};

#endif
