#ifndef EVENTS_VEHICLE_CONTROLLER_EVENT_HPP
#define EVENTS_VEHICLE_CONTROLLER_EVENT_HPP

#include "../Common.hpp"

enum class VehicleControllerAction {
    Attach,
    Detach
};

struct VehicleControllerEvent {
    entt::entity entity;
    VehicleControllerAction action;
};

#endif // EVENTS_VEHICLE_CONTROLLER_EVENT_HPP
