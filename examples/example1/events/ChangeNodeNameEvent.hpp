#ifndef EVENTS_CHANGE_NODE_NAME_EVENT_HPP
#define EVENTS_CHANGE_NODE_NAME_EVENT_HPP

#include "../Common.hpp"

struct ChangeNodeNameEvent {
    hashed_string_owner oldName;
    hashed_string_owner newName;
};

#endif // EVENTS_CHANGE_NODE_NAME_EVENT_HPP
