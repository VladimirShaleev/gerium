#ifndef COMPONENTS_NODE_HPP
#define COMPONENTS_NODE_HPP

#include "../Common.hpp"

struct Node {
    entt::entity parent{ entt::null };
    std::vector<entt::entity> childs;
};

#endif
