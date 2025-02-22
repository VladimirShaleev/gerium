#ifndef COMPONENTS_WHEEL_HPP
#define COMPONENTS_WHEEL_HPP

#include "../Common.hpp"

enum class WheelPosition {
    FrontLeft,
    FrontRight,
    BackLeft1,
    BackRight1,
    BackLeft2,
    BackRight2
};

struct Wheel {
    entt::entity parent;
    WheelPosition position;
    glm::vec3 point;
};

#endif
