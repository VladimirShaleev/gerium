#ifndef COMPONENTS_COLLIDER_HPP
#define COMPONENTS_COLLIDER_HPP

#include "../Common.hpp"

struct Collider {
    enum class Shape { Box, Sphere, Capsule } shape;
    glm::vec3 size{1.0f};
};

#endif
