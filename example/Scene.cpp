#include "Scene.hpp"

#include <queue>
#include <stack>

SceneNode* Scene::root() {
    if (!_root) {
        _registry      = entt::registry();
        _root          = allocateNode();
        _root->_entity = _registry.create();
    }
    return _root;
}

SceneNode* Scene::addNode(SceneNode* parent) {
    assert(parent);
    auto node = allocateNode();
    if (parent) {
        node->_parent = parent;
        parent->_childrens.push_back(node);
    }
    node->_entity = _registry.create();
    return node;
}

void Scene::update() {
    auto rootMat = glm::identity<glm::mat4>();
    std::queue<std::tuple<glm::mat4*, bool, SceneNode*>> nodes;
    nodes.push({ &rootMat, false, root() });

    while (!nodes.empty()) {
        auto& [mat, changed, node] = nodes.front();
        nodes.pop();

        auto resultMat = mat;
        if (auto transform = getComponentNode<Transform>(node)) {
            if (transform->changed || changed) {
                transform->worldMatrix = *mat * transform->localMatrix;
                transform->changed     = false;
                changed                = true;
            }
            resultMat = &transform->worldMatrix;
        }

        if (auto obj = getComponentNode<Object>(node); obj && changed) {
            auto parentNodeIndex = obj->model.getParentNodeIndex();
            obj->model.setNodeMatrix(parentNodeIndex, *resultMat);
            obj->model.updateMatrices();
        }

        for (auto& child : node->childrens()) {
            nodes.push({ resultMat, changed, child });
        }
    }
}

SceneNode* Scene::allocateNode() {
    _nodes.push_back(std::make_shared<SceneNode>());
    return _nodes.back().get();
}
