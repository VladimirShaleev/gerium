#ifndef EVENTS_CHANGE_RIGID_BODY_EVENT_HPP
#define EVENTS_CHANGE_RIGID_BODY_EVENT_HPP

#include "../Common.hpp"
#include "../components/RigidBody.hpp"

struct ChangeRigidBodyEvent {
    entt::entity entity;
    std::optional<RigidBody> rigidBody;
};

#endif // EVENTS_CHANGE_RIGID_BODY_EVENT_HPP
