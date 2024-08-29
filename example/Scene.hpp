#ifndef SCENE_HPP
#define SCENE_HPP

#include "BVHNode.hpp"
#include "Camera.hpp"
#include "EntityComponentSystem.hpp"
#include "Model.hpp"

class Transform : public Component {
public:
    Transform() : localMatrix(glm::identity<glm::mat4>()), updated(true) {
    }

    explicit Transform(const glm::mat4& mat) : localMatrix(mat), updated(true) {
    }

    glm::mat4 localMatrix;
    glm::mat4 worldMatrix;
    bool updated;

    void update(Entity& entity, gerium_data_t data) override {
    }
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
    Entity _entity;
};

struct MeshInstance {
    Mesh* mesh{};
    int count{};
    gerium_buffer_h datas{ UndefinedHandle };
    gerium_descriptor_set_h textureSet{ UndefinedHandle };
    MeshData* ptr{};
};

class Scene {
public:
    void create(ResourceManager* resourceManger);

    SceneNode* root();
    SceneNode* addNode(SceneNode* parent);

    void update();
    void culling();
    void clear();

    template <typename T>
    T* addComponentToNode(SceneNode* node, const T& component) {
        return _registry.addComponent(node->_entity, component);
    }

    template <typename T>
    T* getComponentNode(SceneNode* node) noexcept {
        return _registry.getComponent<T>(node->_entity);
    }

    template <typename T>
    void getComponents(gerium_uint16_t& count, T** results) noexcept {
        _registry.getComponents<T>(count, results);
    }

    template <typename T>
    T* getAnyComponentNode() noexcept {
        gerium_uint16_t count = 1;
        T* results[1];
        _registry.getComponents<T>(count, results);
        return count ? results[0] : nullptr;
    }

    Camera* getActiveCamera() noexcept {
        Camera* camera[10];
        gerium_uint16_t count = 10;
        _registry.getComponents<Camera>(count, camera);
        for (gerium_uint16_t i = 0; i < count; ++i) {
            if (camera[i]->isActive()) {
                return camera[i];
            }
        }
        return nullptr;
    }

    const std::vector<Mesh*>& visibleMeshes() const noexcept {
        return _visibleMeshes;
    }

    const std::vector<MeshInstance*>& instances() const noexcept {
        return _instancesLinear;
    }

private:
    static constexpr int kMaxDraws = 500;

    SceneNode* allocateNode();

    ResourceManager* _resourceManger{};
    Registry<Camera, Transform, Model> _registry{};
    std::vector<std::shared_ptr<SceneNode>> _nodes{};
    SceneNode* _root{};
    BVHNode* _bvh{};
    std::vector<Mesh*> _visibleMeshes{};
    std::array<Buffer, kMaxDraws> _meshDatas{};
    std::array<DescriptorSet, kMaxDraws> _textureSets{};
    std::unordered_map<gerium_uint64_t, MeshInstance> _instances{};
    std::vector<MeshInstance*> _instancesLinear{};
    SceneData _sceneData{};
};

#endif
