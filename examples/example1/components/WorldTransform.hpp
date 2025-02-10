#ifndef COMPONENTS_WORLD_TRANSFORM_HPP
#define COMPONENTS_WORLD_TRANSFORM_HPP

#include "../Common.hpp"

struct WorldTransform {
    glm::mat4 matrix{ 1.0f };
    float scale{ 1.0f };
};

#endif
