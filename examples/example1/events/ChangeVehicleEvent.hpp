#ifndef EVENTS_CHANGE_VEHICLE_EVENT_HPP
#define EVENTS_CHANGE_VEHICLE_EVENT_HPP

#include "../Common.hpp"
#include "../components/Vehicle.hpp"

struct ChangeVehicleEvent {
    entt::entity entity;
    std::optional<Vehicle> vehicle;
};

#endif // EVENTS_CHANGE_VEHICLE_EVENT_HPP
