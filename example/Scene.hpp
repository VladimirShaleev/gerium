#ifndef SCENE_HPP
#define SCENE_HPP

#include "Model.hpp"

#include <entt/entt.hpp>

struct SceneData {
    glm::mat4 viewProjection;
    glm::vec4 eye;
};

struct MeshData {
    glm::mat4 world;
    glm::mat4 inverseWorld;
};

struct Transform {
    glm::mat4 localMatrix;
    glm::mat4 worldMatrix;
    bool updated;
};

class SceneNode {
public:
    SceneNode* parent() noexcept {
        return _parent;
    }

    const std::vector<SceneNode*> childrens() const noexcept {
        return _childrens;
    }

private:
    friend class Scene;

    SceneNode* _parent{};
    std::vector<SceneNode*> _childrens;
    entt::registry::entity_type _entity;
};

class Scene {
public:
    SceneNode* root();
    SceneNode* addNode(SceneNode* parent);

    void update();
    void clear();

    template <typename T>
    T& addComponentToNode(SceneNode* node, const T& component) {
        return _registry.emplace<T>(node->_entity, component);
    }

    template <typename T>
    T* getComponentNode(SceneNode* node) noexcept {
        return _registry.try_get<T>(node->_entity);
    }

private:
    SceneNode* allocateNode();

    entt::registry _registry{};
    std::vector<std::shared_ptr<SceneNode>> _nodes{};
    SceneNode* _root{};
};

#endif