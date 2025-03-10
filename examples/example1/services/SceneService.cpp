#include "SceneService.hpp"
#include "../Application.hpp"
#include "../components/Camera.hpp"
#include "../components/Collider.hpp"
#include "../components/Node.hpp"
#include "../components/Renderable.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Static.hpp"
#include "../components/Transform.hpp"
#include "../components/Vehicle.hpp"
#include "../components/VehicleController.hpp"
#include "../components/Wheel.hpp"
#include "TimeService.hpp"

using namespace entt::literals;

static glm::mat4 calcMatrix(const glm::vec3& translate, const glm::quat& rotation, const glm::vec3& scale) {
    auto matT = glm::translate(glm::identity<glm::mat4>(), translate);
    auto matR = glm::mat4_cast(rotation);
    auto matS = glm::scale(glm::identity<glm::mat4>(), scale);
    return matT * matR * matS;
}

void SceneService::onAddModel(const AddModelEvent& event) {
    auto& registry = entityRegistry();

    auto parent = event.parent == entt::null ? _nodes.at(ROOT) : event.parent;
    auto root   = registry.create();

    const auto& name = registry.emplace<Name>(root, event.name);
    checkAndAddNode(root, name);

    registry.emplace<Node>(root).parent = parent;
    registry.get_or_emplace<Node>(parent).childs.push_back(root);

    const auto& model           = getModel(event.model);
    const auto& parentTransform = registry.get_or_emplace<Transform>(parent);
    auto localMatrix            = calcMatrix(event.position, event.rotation, event.scale);
    auto worldMatrix            = parentTransform.matrix * localMatrix;
    auto worldScale             = parentTransform.scale * event.scale;

    auto& rootTransform      = registry.emplace<Transform>(root);
    rootTransform.matrix     = worldMatrix;
    rootTransform.prevMatrix = rootTransform.matrix;
    rootTransform.scale      = worldScale;

    struct Hierarchy {
        gerium_sint32_t nodeIndex;
        gerium_sint32_t childLevel;
        entt::entity parent;
        glm::mat4 parentMatrix;
        glm::vec3 parentScale;
    };

    auto isStatic        = true;
    entt::entity vehicle = entt::null;

    std::queue<Hierarchy> nodesToVisit;
    for (gerium_sint32_t i = 0; i < (gerium_sint32_t) model.nodes.size(); ++i) {
        if (model.nodes[i].parent < 0) {
            nodesToVisit.emplace(i, model.nodes[i].level + 1, root, worldMatrix, worldScale);
        }
    }

    while (!nodesToVisit.empty()) {
        const auto [nodeIndex, childLevel, parent, parentMatrix, parentScale] = nodesToVisit.front();
        nodesToVisit.pop();

        const auto& modelNode = model.nodes[nodeIndex];

        auto node       = registry.create();
        auto& nodeData  = registry.emplace<Node>(node);
        nodeData.name   = modelNode.name;
        nodeData.parent = parent;
        registry.get_or_emplace<Node>(parent).childs.push_back(node);

        const auto& localPosition = modelNode.position;
        const auto& localRotation = modelNode.rotation;
        const auto& localScale    = modelNode.scale;

        auto& transform      = registry.emplace<Transform>(node);
        transform.matrix     = parentMatrix * calcMatrix(localPosition, localRotation, localScale);
        transform.prevMatrix = transform.matrix;
        transform.scale      = parentScale * localScale;

        if (modelNode.name == "vehicle"_hs) {
            isStatic = false;
            vehicle  = node;
            registry.emplace<Vehicle>(node);
        } else if (vehicle != entt::null) {
            std::optional<WheelPosition> wheelPos = std::nullopt;
            switch (modelNode.name) {
                case "wheel_lf"_hs:
                    wheelPos = WheelPosition::FrontLeft;
                    break;
                case "wheel_rf"_hs:
                    wheelPos = WheelPosition::FrontRight;
                    break;
                case "wheel_lb"_hs:
                    wheelPos = WheelPosition::BackLeft1;
                    break;
                case "wheel_rb"_hs:
                    wheelPos = WheelPosition::BackRight1;
                    break;
                case "wheel_lb2"_hs:
                    wheelPos = WheelPosition::BackLeft2;
                    break;
                case "wheel_rb2"_hs:
                    wheelPos = WheelPosition::BackRight2;
                    break;
            }

            if (wheelPos) {
                auto& wheel    = registry.emplace<Wheel>(node);
                wheel.parent   = vehicle;
                wheel.position = wheelPos.value();
                wheel.point    = localPosition * localScale;
                registry.get<Vehicle>(vehicle).wheels.push_back(node);
            }
        }

        if (modelNode.mass != 0.0f) {
            isStatic = false;
        }

        if (isStatic) {
            registry.emplace<Static>(node);
        }

        for (const auto& mesh : model.meshes) {
            if (mesh.nodeIndex == nodeIndex) {
                auto& collider = registry.get_or_emplace<Collider>(node);
                auto& body     = registry.get_or_emplace<RigidBody>(node);
                body.mass      = modelNode.mass;

                switch (modelNode.colliderShape) {
                    case Shape::ConvexHull:
                        collider.shape = Shape::ConvexHull;
                        collider.index = modelNode.colliderIndex;
                        break;
                    case Shape::Mesh:
                        collider.shape = Shape::Mesh;
                        collider.index = modelNode.colliderIndex;
                        break;
                    default:
                        collider.shape      = Shape::Box;
                        collider.halfExtent = (modelNode.bbox.max() - modelNode.bbox.min()) * 0.5f;
                        break;
                }

                auto& renderable = registry.get_or_emplace<Renderable>(node);
                renderable.meshes.push_back({});

                static int ii  = 0;
                auto& meshData = renderable.meshes.back();
                meshData.model = model.name;
                meshData.mesh  = mesh.meshIndex;
                meshData.node  = (gerium_uint32_t) mesh.nodeIndex;
                if (!model.materials.empty()) {
                    meshData.material      = model.materials[mesh.materialIndex];
                    meshData.material.name = (ii++) % 2 == 0 ? TECH_PBR_ID : TECH_OTHER_ID; // TODO: remove
                }
            }
        }

        for (gerium_sint32_t i = 0; i < (gerium_sint32_t) model.nodes.size(); ++i) {
            if (model.nodes[i].level == childLevel && model.nodes[i].parent == nodeIndex) {
                nodesToVisit.emplace(i, model.nodes[i].level + 1, node, transform.matrix, transform.scale);
            }
        }
    }
}

void SceneService::onAddNodeName(const AddNodeNameEvent& event) {
    if (entityRegistry().any_of<Name>(event.entity)) {
        throw std::runtime_error("Node already added component Name");
    }
    entityRegistry().emplace<Name>(event.entity).name = event.name;
    checkAndAddNode(event.entity, { event.name });
}

void SceneService::onChangeNodeName(const ChangeNodeNameEvent& event) {
    if (auto it = _nodes.find(event.oldName); it != _nodes.end()) {
        auto entity = it->second;
        _nodes.erase(it);
        if (event.newName != ""_hs) {
            _nodes[event.newName]                   = entity;
            entityRegistry().get<Name>(entity).name = event.newName;
        } else {
            entityRegistry().erase<Name>(entity);
        }
    }
}

void SceneService::onDeleteNode(const DeleteNodeEvent& event) {
    auto& registry = entityRegistry();

    auto parent        = registry.get<Node>(event.entity).parent;
    auto& parentChilds = registry.get<Node>(parent).childs;
    parentChilds.erase(std::find(parentChilds.begin(), parentChilds.end(), event.entity));

    std::queue<entt::entity> visit;
    visit.push(event.entity);
    while (!visit.empty()) {
        auto entity = visit.front();
        visit.pop();

        for (auto child : registry.get<Node>(entity).childs) {
            visit.push(child);
        }

        if (auto name = registry.try_get<Name>(entity)) {
            if (auto it = _nodes.find(name->name); it != _nodes.end()) {
                _nodes.erase(it);
            }
        }
        registry.destroy(entity);
    }
}

void SceneService::onDeleteNodeByName(const DeleteNodeByNameEvent& event) {
    if (auto it = _nodes.find(event.name); it != _nodes.end()) {
        onDeleteNode({ it->second });
    }
}

void SceneService::onTransformNode(const TransformNodeEvent& event) {
    auto& transform = entityRegistry().get<Transform>(event.entity);
    if (event.transformChilds && !_transformChanges.contains(event.entity)) {
        auto parent = entityRegistry().get<Node>(event.entity).parent;
        auto parentMatrix = parent != entt::null
            ? entityRegistry().get<Transform>(parent).matrix
            : glm::identity<glm::mat4>();
        auto parentScale = parent != entt::null
            ? entityRegistry().get<Transform>(parent).scale
            : glm::vec3(1.0f);
        
        std::queue<std::tuple<entt::entity, glm::mat4, glm::vec3>> visit;
        visit.emplace(event.entity, glm::inverse(parentMatrix), 1.0f / parentScale);
        
        while (!visit.empty()) {
            auto [entity, parentInvOldMatrix, parentInvOldScale] = visit.front();
            visit.pop();
            
            if (auto it = _transformChanges.find(entity); it == _transformChanges.end()) {
                auto& t = entityRegistry().get<Transform>(entity);
                t.prevMatrix = t.matrix;
                _transformChanges[entity] = { parentInvOldMatrix, parentInvOldScale };
                for (const auto child : entityRegistry().get<Node>(entity).childs) {
                    visit.emplace(child, glm::inverse(t.matrix), 1.0f / t.scale);
                }
            }
        }
    }
    transform.matrix = calcMatrix(event.position, event.rotation, event.scale);
    transform.scale  = event.scale;
}

void SceneService::checkAndAddNode(entt::entity entity, const Name& name) {
    if (auto it = _nodes.find(name.name); it == _nodes.end()) {
        _nodes[name.name] = entity;
    } else if (it->second != entity) {
        static std::string message;
        message = "Node with name '" + name.name.string() + "' already exists";
        throw std::runtime_error(message.c_str());
    }
}

const Model& SceneService::getModel(const entt::hashed_string& modelId) {
    if (auto it = _models.find(modelId); it != _models.end()) {
        return it->second;
    }
    _models[modelId] = loadModel(modelId);
    return _models[modelId];
}

void SceneService::start() {
    for (auto [entity, name] : entityRegistry().view<Name>().each()) {
        checkAndAddNode(entity, name);
    }
    if (auto it = _nodes.find(ROOT); it == _nodes.end()) {
        auto root = entityRegistry().create();
        entityRegistry().emplace<Name>(root, ROOT);
        _nodes[ROOT] = root;
    }
    application().dispatcher().sink<AddModelEvent>().connect<&SceneService::onAddModel>(*this);
    application().dispatcher().sink<AddNodeNameEvent>().connect<&SceneService::onAddNodeName>(*this);
    application().dispatcher().sink<ChangeNodeNameEvent>().connect<&SceneService::onChangeNodeName>(*this);
    application().dispatcher().sink<DeleteNodeEvent>().connect<&SceneService::onDeleteNode>(*this);
    application().dispatcher().sink<DeleteNodeByNameEvent>().connect<&SceneService::onDeleteNodeByName>(*this);
    application().dispatcher().sink<TransformNodeEvent>().connect<&SceneService::onTransformNode>(*this);
}

void SceneService::stop() {
    application().dispatcher().sink<TransformNodeEvent>().disconnect(this);
    application().dispatcher().sink<DeleteNodeByNameEvent>().disconnect(this);
    application().dispatcher().sink<DeleteNodeEvent>().disconnect(this);
    application().dispatcher().sink<ChangeNodeNameEvent>().disconnect(this);
    application().dispatcher().sink<AddNodeNameEvent>().disconnect(this);
    application().dispatcher().sink<AddModelEvent>().disconnect(this);
    _models.clear();
    _nodes.clear();
}

void SceneService::update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) {
    if (!_transformChanges.empty()) {
        updateTransformations();
        _transformChanges.clear();
    }
    
    _models.clear();
    auto view = entityRegistry().view<Camera>();

    gerium_uint16_t width, height;
    gerium_application_get_size(application().handle(), &width, &height);

    if (width == 0 || height == 0) {
        return;
    }

    for (auto entity : view) {
        auto& camera = view.get<Camera>(entity);

        camera.fov   = std::clamp(camera.fov, glm::radians(30.0f), glm::radians(120.0f));
        camera.pitch = std::clamp(camera.pitch, glm::radians(-89.99f), glm::radians(89.99f));
        camera.yaw   = std::fmod(camera.yaw, glm::two_pi<gerium_float32_t>());
        if (camera.yaw < 0.0f) {
            camera.yaw += glm::two_pi<gerium_float32_t>();
        }

        camera.front.x = cos(camera.yaw) * cos(camera.pitch);
        camera.front.y = sin(camera.pitch);
        camera.front.z = sin(camera.yaw) * cos(camera.pitch);
        camera.front   = glm::normalize(camera.front);
        camera.right   = glm::normalize(glm::cross(camera.front, glm::vec3(0.0f, 1.0f, 0.0f)));
        camera.up      = glm::normalize(glm::cross(camera.right, camera.front));

        const auto aspect = float(width) / height;

        const auto projection = glm::perspective(camera.fov, aspect, camera.nearPlane, camera.farPlane);

        camera.resolution         = { width, height };
        camera.prevView           = camera.view;
        camera.prevProjection     = camera.projection;
        camera.prevViewProjection = camera.viewProjection;
        camera.projection         = projection;
        camera.view               = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
        camera.viewProjection     = camera.projection * camera.view;
    }
}

void SceneService::updateTransformations() {
    for (const auto& [entity, inv] : _transformChanges) {
        auto parent = entityRegistry().get<Node>(entity).parent;
        auto parentMatrix = parent != entt::null
            ? entityRegistry().get<Transform>(parent).matrix
            : glm::identity<glm::mat4>();
        auto parentScale = parent != entt::null
            ? entityRegistry().get<Transform>(parent).scale
            : glm::vec3(1.0f);
        auto& transform = entityRegistry().get<Transform>(entity);
        const auto localMatrix = inv.invParentMatrix * transform.matrix;
        const auto localScale  = inv.invParentScale * transform.scale;
        transform.matrix = parentMatrix * localMatrix;
        transform.scale  = parentScale * localScale;
        entityRegistry().replace<Transform>(entity, transform);
    }
    _transformChanges.clear();
}
