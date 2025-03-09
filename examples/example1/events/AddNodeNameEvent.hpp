#ifndef EVENTS_ADD_NODE_NAME_EVENT_HPP
#define EVENTS_ADD_NODE_NAME_EVENT_HPP

#include "../Common.hpp"

struct AddNodeNameEvent {
    entt::entity entity;
    hashed_string_owner name;
};

#endif // EVENTS_ADD_NODE_NAME_EVENT_HPP
