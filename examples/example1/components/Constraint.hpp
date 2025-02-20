#ifndef COMPONENTS_CONSTRAINT_HPP
#define COMPONENTS_CONSTRAINT_HPP

#include "../Common.hpp"

struct Constraint {
    entt::entity parent;
    glm::vec3 point;
    glm::vec3 axis;
};

#endif
