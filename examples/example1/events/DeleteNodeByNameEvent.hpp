#ifndef EVENTS_DELETE_NODE_BY_NAME_EVENT_HPP
#define EVENTS_DELETE_NODE_BY_NAME_EVENT_HPP

#include "../Common.hpp"

struct DeleteNodeByNameEvent {
    hashed_string_owner name;
};

#endif // EVENTS_DELETE_NODE_BY_NAME_EVENT_HPP
