#ifndef EVENTS_TRANSFORM_NODE_EVENT_HPP
#define EVENTS_TRANSFORM_NODE_EVENT_HPP

#include "../Common.hpp"

struct TransformNodeEvent {
    entt::entity entity;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    bool transformChilds;
};

#endif // EVENTS_TRANSFORM_NODE_EVENT_HPP
