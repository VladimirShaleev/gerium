#ifndef SCENE_HPP
#define SCENE_HPP

#include "Model.hpp"

#include <entt/entt.hpp>

struct Transform {
    glm::mat4 localMatrix;
    glm::mat4 worldMatrix;
    bool changed;
};

struct Object {
    Model model;
    gerium_technique_h technique;
    gerium_descriptor_set_h descriptorSet;
    gerium_buffer_h data;
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

    template <typename T>
    void addComponentToNode(SceneNode* node, const T& component) {
        _registry.emplace<T>(node->_entity, component);
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
