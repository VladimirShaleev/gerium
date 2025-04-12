#ifndef EVENTS_CHANGE_COLLIDER_EVENT_HPP
#define EVENTS_CHANGE_COLLIDER_EVENT_HPP

#include "../Common.hpp"
#include "../components/Collider.hpp"

struct ChangeColliderEvent {
    entt::entity entity;
    std::optional<Collider> collider;
};

#endif // EVENTS_CHANGE_COLLIDER_EVENT_HPP
