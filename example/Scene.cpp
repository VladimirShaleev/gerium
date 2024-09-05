#include "Scene.hpp"

#include <queue>
#include <stack>

void Scene::create(ResourceManager* resourceManger, bool bindlessEnabled) {
    _resourceManger  = resourceManger;
    _bindlessEnabled = bindlessEnabled;

    const auto meshDataSize = _bindlessEnabled ? sizeof(MeshDataBindless) : sizeof(MeshData);

    _meshDatas = _resourceManger->createBuffer(
        GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "", "mesh_data", nullptr, meshDataSize * kMaxMeshDatas);

    for (auto& set : _textureSets) {
        set = _resourceManger->createDescriptorSet();
    }
}

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

    _instances.clear();
    _instancesLinear.clear();

    auto renderer = _resourceManger->renderer();
    auto countMeshes = 0;

    for (const auto mesh : _visibleMeshes) {
        if (_instances.size() == kMaxDraws) {
            break;
        }
        auto& meshInstance = _instances[mesh->hash()];
        if (!meshInstance.mesh) {
            meshInstance.mesh = mesh;
            if (!_bindlessEnabled) {
                meshInstance.textureSet = _textureSets[_instances.size() - 1];
                gerium_renderer_bind_texture(renderer, meshInstance.textureSet, 0, 0, mesh->getMaterial().getDiffuse());
                gerium_renderer_bind_texture(renderer, meshInstance.textureSet, 1, 0, mesh->getMaterial().getNormal());
                gerium_renderer_bind_texture(
                    renderer, meshInstance.textureSet, 2, 0, mesh->getMaterial().getRoughness());
            }
        }
        if (meshInstance.meshDatas.size() > MAX_INSTANCES) {
            continue;
        }
        if (countMeshes > kMaxMeshDatas) {
            break;
        }
        meshInstance.meshDatas.push_back(&mesh->getMaterial().meshData());
        ++countMeshes;
    }

    auto mapPtr = gerium_renderer_map_buffer(renderer, _meshDatas, 0, 0);
    MeshDataBindless* bindlessPtr{};
    MeshData* ptr{};
    if (_bindlessEnabled) {
        bindlessPtr = (MeshDataBindless*) mapPtr;
    } else {
        ptr = (MeshData*) mapPtr;
    }
    gerium_uint16_t offset = 0;
    for (auto& [_, instance] : _instances) {
        instance.first = offset;
        instance.count = (gerium_uint16_t) instance.meshDatas.size();
        offset += instance.count;

        for (auto meshData : instance.meshDatas) {
            if (_bindlessEnabled) {
                *((MeshData*) bindlessPtr) = *meshData;
                bindlessPtr->textures.x    = ((gerium_texture_h) instance.mesh->getMaterial().getDiffuse()).unused;
                bindlessPtr->textures.y    = ((gerium_texture_h) instance.mesh->getMaterial().getNormal()).unused;
                bindlessPtr->textures.z    = ((gerium_texture_h) instance.mesh->getMaterial().getRoughness()).unused;
                bindlessPtr->textures.w    = UndefinedHandle;
                ++bindlessPtr;
            } else {
                *ptr = *meshData;
                ++ptr;
            }
        }
    }
    gerium_renderer_unmap_buffer(renderer, _meshDatas);

    _instancesLinear.resize(_instances.size());

    int i = 0;
    for (auto& [_, instance] : _instances) {
        _instancesLinear[i++] = &instance;
    }
}

void Scene::clear() {
    _instances.clear();
    for (auto& set : _textureSets) {
        set = nullptr;
    }
    _meshDatas = nullptr;
    _registry.clear();
    _nodes.clear();
    _root = nullptr;
}

SceneNode* Scene::allocateNode() {
    _nodes.push_back(std::make_shared<SceneNode>());
    return _nodes.back().get();
}
