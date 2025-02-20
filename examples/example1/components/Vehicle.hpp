#ifndef COMPONENTS_VEHICLE_HPP
#define COMPONENTS_VEHICLE_HPP

#include "../Common.hpp"

struct Vehicle {
    std::vector<entt::entity> wheels;
    gerium_float32_t maxRollAngle{ PI };
};

#endif
