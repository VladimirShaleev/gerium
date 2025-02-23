#ifndef COMPONENTS_COLLIDER_HPP
#define COMPONENTS_COLLIDER_HPP

#include "../Common.hpp"

enum class Shape {
    Box,
    Sphere,
    Capsule,
    Mesh
};

struct Collider {
    Shape shape;
    glm::vec3 halfExtent;
    gerium_float32_t halfHeightOfCylinder;
    gerium_float32_t radius;
    gerium_uint32_t index;
};

#endif
