#ifndef COMPONENTS_CHANGES_HPP
#define COMPONENTS_CHANGES_HPP

#include "../Common.hpp"
#include "Collider.hpp"
#include "Renderable.hpp"
#include "RigidBody.hpp"
#include "Transform.hpp"
#include "Vehicle.hpp"

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

template <typename>
struct ComponentChange;

template <>
struct ComponentChange<Transform> {
    static Change& getChange(Changes& changes) noexcept {
        return changes.transforms;
    }
};

template <>
struct ComponentChange<RigidBody> {
    static Change& getChange(Changes& changes) noexcept {
        return changes.rigidBodies;
    }
};

template <>
struct ComponentChange<Vehicle> {
    static Change& getChange(Changes& changes) noexcept {
        return changes.vehicles;
    }
};

template <>
struct ComponentChange<Collider> {
    static Change& getChange(Changes& changes) noexcept {
        return changes.colliders;
    }
};

template <>
struct ComponentChange<Renderable> {
    static Change& getChange(Changes& changes) noexcept {
        return changes.renderables;
    }
};

template <typename C>
inline Change& getChangeFlags(Changes& changes) noexcept {
    return ComponentChange<C>::getChange(changes);
}

template <typename C>
inline Change& setChangeFlags(Changes& changes, Change flags) noexcept {
    auto& value = getChangeFlags<C>(changes);
    value |= flags;
    return value;
}

template <typename C>
inline bool checkAllChangeFlags(Changes& changes, Change flags) noexcept {
    return (getChangeFlags<C>(changes) & flags) == flags;
}

template <typename C>
inline bool checkAnyChangeFlags(Changes& changes, Change flags) noexcept {
    return (getChangeFlags<C>(changes) & flags) != Change::None;
}

#endif
