#ifndef EVENTS_MOVE_NODE_EVENT_HPP
#define EVENTS_MOVE_NODE_EVENT_HPP

#include "../Common.hpp"

struct MoveNodeEvent {
    entt::entity entity;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    bool transformChilds;
};

#endif // EVENTS_MOVE_NODE_EVENT_HPP
