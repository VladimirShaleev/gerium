#include "Scene.hpp"

#include <queue>
#include <stack>

SceneNode* Scene::root() {
    if (!_root) {
        _root = allocateNode();
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
    return node;
}

void Scene::update() {
    auto rootMat = glm::identity<glm::mat4>();
    std::queue<std::tuple<glm::mat4*, bool, SceneNode*>> nodes;
    nodes.push({ &rootMat, false, root() });

    while (!nodes.empty()) {
        auto& [parentMat, parentUpdated, node] = nodes.front();
        nodes.pop();

        auto mat     = parentMat;
        auto updated = parentUpdated;

        if (auto transform = getComponentNode<Transform>(node)) {
            if (transform->updated || updated) {
                transform->worldMatrix = *mat * transform->localMatrix;
                transform->updated     = false;
                updated                = true;
            }
            mat = &transform->worldMatrix;
        }

        if (auto model = getComponentNode<Model>(node); model) {
            model->updateMatrices(*mat, updated);
            model->updateMaterials();
        }

        _registry.update(node->_entity, (gerium_data_t) this);

        for (auto& child : node->childrens()) {
            nodes.push({ mat, updated, child });
        }
    }
}

void Scene::clear() {
    _registry.clear();
    _nodes.clear();
    _root = nullptr;
}

SceneNode* Scene::allocateNode() {
    _nodes.push_back(std::make_shared<SceneNode>());
    return _nodes.back().get();
}
