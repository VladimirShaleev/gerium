#ifndef COMPONENTS_WHEEL_HPP
#define COMPONENTS_WHEEL_HPP

#include "../Common.hpp"

struct Wheel {
    enum Position {
        FrontLeft,
        FrontRight,
        BackLeft1,
        BackRight1,
        BackLeft2,
        BackRight2
    };

    entt::entity parent;
    glm::vec3 point;
    Position position;
    JPH::uint id;
};

#endif
