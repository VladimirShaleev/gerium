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

    bool transformUpdated = false;

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
                transformUpdated       = true;
            }
            mat = &transform->worldMatrix;
        }

        if (auto model = getComponentNode<Model>(node); model) {
            if (model->updateMatrices(*mat, updated)) {
                transformUpdated = true;
            }
            model->updateMaterials();
        }

        _registry.update(node->_entity, (gerium_data_t) this);

        for (auto& child : node->childrens()) {
            nodes.push({ mat, updated, child });
        }
    }

    if (transformUpdated) {
        if (_bvh) {
            delete _bvh;
        }
        _bvh = BVHNode::build(*this);
    }
}

void Scene::culling() {
    auto camera = getAnyComponentNode<Camera>();

    _visibleMeshes.clear();

    std::function<void(const BVHNode*)> cullingMesh;
    cullingMesh = [this, camera, &cullingMesh](const BVHNode* node) {
        if (camera->test(node->bbox()) == Intersection::None) {
            return;
        }
        if (node->leaf()) {
            for (auto mesh : node->meshes()) {
                if (camera->test(mesh->worldBoundingBox()) != Intersection::None) {
                    mesh->visible(true);
                    _visibleMeshes.push_back(mesh);
                } else {
                    mesh->visible(false);
                }
            }
        }
        if (node->left()) {
            cullingMesh(node->left());
        }
        if (node->right()) {
            cullingMesh(node->right());
        }
    };
    cullingMesh(_bvh);
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
