#ifndef EVENTS_ADD_MODEL_EVENT_HPP
#define EVENTS_ADD_MODEL_EVENT_HPP

#include "../Common.hpp"

struct AddModelEvent {
    hashed_string_owner parent;
    hashed_string_owner name;
    entt::hashed_string model;
    glm::vec3 position{ 0.0f, 0.0f, 0.0f };
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
};

#endif // EVENTS_ADD_MODEL_EVENT_HPP
