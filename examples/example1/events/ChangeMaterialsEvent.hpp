#ifndef EVENTS_CHANGE_MATERIALS_EVENT_HPP
#define EVENTS_CHANGE_MATERIALS_EVENT_HPP

#include "../Common.hpp"
#include "../components/Renderable.hpp"

struct ChangeMaterialsEvent {
    entt::entity entity;
    std::vector<MaterialData> materials;
};

#endif // EVENTS_CHANGE_MATERIALS_EVENT_HPP
