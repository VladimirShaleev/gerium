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

class Light : public Component {
public:
    enum Type {
        None,
        Point
    };

    Light() = default;

    Light(const PointLight& light) noexcept : _type(Point), _pointLight(light) {
    }

    void update(Entity& entity, gerium_data_t data) override {
    }

    Type type() const noexcept {
        return _type;
    }

    bool updateBbox(const glm::mat4& parentMat = glm::identity<glm::mat4>(), bool parentUpdated = false) {
        if (_changed || parentUpdated) {
            if (_type == Point) {
                const auto p1 = _pointLight.position.xyz() - _pointLight.attenuation;
                const auto p2 = _pointLight.position.xyz() + _pointLight.attenuation;
                _bbox         = BoundingBox({ std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z) },
                                            { std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z) });
                _worldBbox =
                    BoundingBox(parentMat * glm::vec4(_bbox.min(), 1.0f), parentMat * glm::vec4(_bbox.max(), 1.0f));
            }
            _changed = false;
            return true;
        }
        return false;
    }

    const BoundingBox& boundingBox() const noexcept {
        return _bbox;
    }

    const BoundingBox& worldBoundingBox() const noexcept {
        return _worldBbox;
    }

    const PointLight& pointLight() const noexcept {
        return _pointLight;
    }

private:
    bool _changed{ true };
    BoundingBox _bbox{};
    BoundingBox _worldBbox{};
    Type _type{ None };

    union {
        PointLight _pointLight{};
    };
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

struct MeshDataBindless : MeshData {
    glm::uvec4 textures;
};

struct MeshInstance {
    Mesh* mesh{};
    gerium_uint16_t first{};
    gerium_uint16_t count{};
    std::vector<const MeshData*> meshDatas{};
    gerium_descriptor_set_h textureSet{ UndefinedHandle };
};

struct SortedLight {
    gerium_uint32_t index;
    gerium_float32_t projectedZ;
    gerium_float32_t projectedZMin;
    gerium_float32_t projectedZMax;
};

class Scene {
public:
    void create(ResourceManager* resourceManger, bool bindlessEnabled);

    SceneNode* root();
    SceneNode* addNode(SceneNode* parent);

    void update();
    void culling();
    void clear();
    void updateLightTilesSize(gerium_uint16_t width, gerium_uint16_t height);

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
        if (count > 0) {
            camera[0]->activate();
            return camera[0];
        }
        return nullptr;
    }

    const std::vector<Mesh*>& visibleMeshes() const noexcept {
        return _visibleMeshes;
    }

    const gerium_uint32_t getNumTechniques() const noexcept {
        return (gerium_uint32_t) _techniques.size();
    }

    const std::vector<MeshInstance*>& instances() const noexcept {
        return _instancesLinear;
    }

    const gerium_buffer_h getMeshDatas() const noexcept {
        return _meshDatas;
    }

    const gerium_descriptor_set_h getBindlessTextures() const noexcept {
        return _bindlessTextures;
    }

    const gerium_descriptor_set_h lightSet() const noexcept {
        return _lightSet;
    }

private:
    static constexpr int kMaxDraws     = 500;
    static constexpr int kMaxMeshDatas = 1000;

    SceneNode* allocateNode();

    ResourceManager* _resourceManger{};
    bool _bindlessEnabled{};
    Registry<Camera, Transform, Light, Model> _registry{};
    std::vector<std::shared_ptr<SceneNode>> _nodes{};
    SceneNode* _root{};
    BVHNode* _bvh{};
    std::vector<Mesh*> _visibleMeshes{};
    std::vector<Light*> _visibleLights{};
    std::array<gerium_uint32_t, LIGHT_Z_BINS> _lightsLUT{};
    DescriptorSet _bindlessTextures{};
    Texture _emptyTexture{};
    Buffer _meshDatas{};
    std::array<DescriptorSet, kMaxDraws> _textureSets{};
    std::unordered_map<gerium_uint64_t, MeshInstance> _instances{};
    std::vector<MeshInstance*> _instancesLinear{};
    std::set<gerium_uint16_t> _techniques{};
    Buffer _lights{};
    Buffer _lightIndices{};
    Buffer _lightDataLUT{};
    Buffer _lightTiles{};
    DescriptorSet _lightSet{};
};

#endif
