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
    bool changed;
};

struct Object {
    Model model;
    //gerium_renderer_t renderer;
    //gerium_technique_h technique;
    //gerium_descriptor_set_h descriptorSet;
    //gerium_buffer_h data;
    //bool initialized;

    // void destroy() {
    //     if (initialized) {
    //         gerium_renderer_destroy_descriptor_set(renderer, descriptorSet);
    //         gerium_renderer_destroy_buffer(renderer, data);
    //     }
    // }

    // void init() {
    //     if (!initialized) {
    //         check(gerium_renderer_create_buffer(
    //             renderer, GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "mesh_data", nullptr, sizeof(MeshData), &data));
    //         check(gerium_renderer_create_descriptor_set(renderer, &descriptorSet));
    //         initialized = true;
    //     }
    // }
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

    void destroy() {
        // for (auto [_, obj] : _registry.view<Object>().each()) {
        //     obj.destroy();
        // }
    }

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
