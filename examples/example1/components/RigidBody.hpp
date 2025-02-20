#ifndef COMPONENTS_RIGID_BODY_HPP
#define COMPONENTS_RIGID_BODY_HPP

#include "../Common.hpp"

struct RigidBody {
    gerium_float32_t mass{ 1.0f };
    gerium_float32_t linearDamping{ 0.2f };
    gerium_float32_t angularDamping{ 0.2f };
    bool isKinematic{ false };
    JPH::BodyID body{};
};

#endif
