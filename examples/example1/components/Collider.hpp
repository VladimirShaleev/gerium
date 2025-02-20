#ifndef COMPONENTS_COLLIDER_HPP
#define COMPONENTS_COLLIDER_HPP

#include "../Common.hpp"

struct Collider {
    enum Shape {
        Box,
        Sphere,
        Capsule,
        Mesh
    } shape;

    union {
        glm::vec3 size;
        gerium_float32_t radius;
        gerium_uint32_t index;
    };
};

#endif
