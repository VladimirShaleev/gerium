#ifndef COMPONENTS_LOCAL_TRANSFORM_HPP
#define COMPONENTS_LOCAL_TRANSFORM_HPP

#include "../Common.hpp"

struct LocalTransform {
    glm::vec3 position{ 0.0f };
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{};
    bool isDirty{ true };
};

#endif
