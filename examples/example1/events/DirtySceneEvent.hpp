#ifndef EVENTS_DIRTY_SCENE_EVENT_HPP
#define EVENTS_DIRTY_SCENE_EVENT_HPP

#include "../Common.hpp"

enum class DirtyFlags {
    None = 0,
    Moved = 1,
    Added = 2,
    Deleted = 4
};
GERIUM_FLAGS(DirtyFlags);

struct DirtySceneEvent {
    DirtyFlags falgs{};
    bool hasStatics{};
};

#endif // EVENTS_DIRTY_SCENE_EVENT_HPP
