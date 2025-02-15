#ifndef COMPONENTS_RIGID_BODY_HPP
#define COMPONENTS_RIGID_BODY_HPP

#include "../Common.hpp"

struct RigidBody {
    float mass{ 1.0f };
    glm::vec3 velocity{ 0.0f };
    glm::vec3 angularVelocity{ 0.0f };
    bool isKinematic{ false };
    JPH::BodyID body{};
};

#endif
