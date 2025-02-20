#ifndef COMPONENTS_TRANSFORM_HPP
#define COMPONENTS_TRANSFORM_HPP

#include "../Common.hpp"

struct Transform {
    glm::mat4 matrix{ 1.0f };
    glm::mat4 prevMatrix{ 1.0f };
    glm::vec3 scale{ 1.0f };
};

#endif
