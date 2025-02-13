#include "SceneService.hpp"
#include "../Application.hpp"
#include "../components/Camera.hpp"
#include "TimeService.hpp"

// Entity SceneService::root() {
//     if (_root == UndefinedNode) {
//         _root = entityRegistry().getOrCreateEntity("_root");
//         entityRegistry().addComponent<Node>(_root);
//         addDefaultPosition(_root);
//     }
//     return _root;
// }
//
// Entity SceneService::addModel(Entity parent, const std::string& name, const Model* model) {
//     auto rootModel = entityRegistry().createEntity(name);
//     addChild(parent, rootModel);
//
//     std::queue<std::pair<gerium_uint32_t, Entity>> nodesToVisit;
//     for (gerium_sint32_t nodeIndex = 0; nodeIndex < model->nodeCount(); ++nodeIndex) {
//         if (model->nodeParent(nodeIndex) < 0) {
//             nodesToVisit.emplace(nodeIndex, rootModel);
//         }
//     }
//
//     while (!nodesToVisit.empty()) {
//         auto [nodeIndex, parentEntity] = nodesToVisit.front();
//         nodesToVisit.pop();
//
//         auto nodeName = std::string(model->nodeName(nodeIndex).data(), model->nodeName(nodeIndex).length());
//         auto entity   = entityRegistry().createEntity(name + '|' + nodeName);
//         addChild(parentEntity, entity);
//
//         const auto& matrix = model->nodeTransform(nodeIndex);
//         glm::vec3 scale;
//         glm::quat rotation;
//         glm::vec3 translation;
//         glm::vec3 skew;
//         glm::vec4 perspective;
//         glm::decompose(matrix, scale, rotation, translation, skew, perspective);
//
//         auto& position       = entityRegistry().getComponent<Position>(entity);
//         position.translate.x = translation.x;
//         position.translate.y = translation.y;
//         position.translate.z = translation.z;
//         position.rotation.x  = rotation.x;
//         position.rotation.y  = rotation.y;
//         position.rotation.z  = rotation.z;
//         position.rotation.w  = rotation.w;
//         position.scale.x     = scale.x;
//         position.scale.y     = scale.y;
//         position.scale.z     = scale.z;
//
//         for (gerium_uint32_t meshIndex = 0; meshIndex < model->meshes().size(); ++meshIndex) {
//             if (model->meshes()[meshIndex].nodeIndex == nodeIndex) {
//                 auto& mesh     = entityRegistry().getComponent<Mesh>(entity, true);
//                 mesh.modelName = model->name();
//                 mesh.meshIndices.push_back(meshIndex);
//             }
//         }
//
//         for (gerium_sint32_t childIndex = 0; childIndex < model->nodeCount(); ++childIndex) {
//             if (model->nodeParent(childIndex) == nodeIndex) {
//                 nodesToVisit.emplace(childIndex, entity);
//             }
//         }
//     }
//
//     return rootModel;
// }
//
// void SceneService::addChild(Entity parent, Entity child) {
//     if (parent == child) {
//         return;
//     }
//
//     if (!entityRegistry().hasComponent<Node>(parent)) {
//         entityRegistry().addComponent<Node>(parent);
//     }
//     if (!entityRegistry().hasComponent<Node>(child)) {
//         entityRegistry().addComponent<Node>(child);
//     }
//     auto& parentNode = entityRegistry().getComponent<Node>(parent);
//     auto& childNode  = entityRegistry().getComponent<Node>(child);
//
//     if (childNode.parent != parent) {
//         removeChild(child);
//         parentNode.childs.push_back(child);
//         childNode.parent = parent;
//         addDefaultPosition(parent);
//         addDefaultPosition(child);
//     }
// }
//
// void SceneService::removeChild(Entity child) {
//     if (child != _root && _root != UndefinedNode) {
//         auto& childNode = entityRegistry().getComponent<Node>(child, true);
//         if (childNode.parent != UndefinedNode) {
//             auto& childs = entityRegistry().getComponent<Node>(childNode.parent).childs;
//             childs.erase(std::find(childs.begin(), childs.end(), child));
//
//             std::queue<Entity> nodesToVisit;
//             nodesToVisit.push(child);
//
//             while (!nodesToVisit.empty()) {
//                 auto entity = nodesToVisit.front();
//                 nodesToVisit.pop();
//
//                 auto node = entityRegistry().getComponent<Node>(entity);
//                 for (auto child : node.childs) {
//                     nodesToVisit.push(child);
//                 }
//
//                 entityRegistry().destroyEntity(entity);
//             }
//         }
//     }
// }
//
// void SceneService::clear() {
//     if (_root == UndefinedNode) {
//         return;
//     }
//
//     std::queue<Entity> nodesToVisit;
//     nodesToVisit.push(_root);
//
//     std::set<Entity> e;
//
//     while (!nodesToVisit.empty()) {
//         auto entity = nodesToVisit.front();
//         nodesToVisit.pop();
//
//         auto node = entityRegistry().getComponent<Node>(entity);
//         for (auto child : node.childs) {
//             nodesToVisit.push(child);
//         }
//
//         entityRegistry().destroyEntity(entity);
//     }
//     _root = UndefinedNode;
// }
//
void SceneService::start() {
}

void SceneService::stop() {
    // clear();
}

void SceneService::update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) {
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

    // static float angle = 90.0f;

    // auto& camera0    = entityRegistry().getComponent<Position>(entityRegistry().getEntity("camera0"));
    // camera0.rotation = glm::angleAxis(glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    // camera0.updated  = true;

    // angle += sin(manager().getService<TimeService>()->elapsed());

    // std::queue<std::pair<Entity, bool>> updates;
    // updates.emplace(root(), false);

    // while (!updates.empty()) {
    //     auto [entity, updated] = updates.front();
    //     updates.pop();

    //     auto& position   = entityRegistry().getComponent<Position>(entity);
    //     position.updated = updated || position.updated;

    //     auto node = entityRegistry().getComponent<Node>(entity);
    //     for (auto child : node.childs) {
    //         updates.emplace(child, position.updated);
    //     }
    // }

    // std::queue<std::pair<Entity, glm::mat4>> nodesToVisit;
    // nodesToVisit.emplace(root(), glm::identity<glm::mat4>());

    // while (!nodesToVisit.empty()) {
    //     const auto [entity, parentTransform] = nodesToVisit.front();
    //     nodesToVisit.pop();

    //     auto& position = entityRegistry().getComponent<Position>(entity);
    //     // if (position.updated) {
    //     position.updated = false;

    //     glm::vec3& scale     = position.scale;
    //     glm::vec3& translate = position.translate;
    //     glm::quat& rotation  = position.rotation;

    //     auto matS = glm::scale(glm::identity<glm::mat4>(), scale);
    //     auto matT = glm::translate(glm::identity<glm::mat4>(), translate);
    //     auto matR = glm::mat4_cast(rotation);
    //     auto mat  = matT * matR * matS;

    //     auto worldMat = parentTransform * mat;

    //     auto hasPrevTransform = entityRegistry().hasComponent<Transform>(entity);

    //     auto& transform = entityRegistry().getComponent<Transform>(entity, true);
    //     if (hasPrevTransform) {
    //         transform.prevWorld = transform.world;
    //     } else {
    //         transform.prevWorld = worldMat;
    //     }
    //     transform.world = worldMat;

    //     auto node = entityRegistry().getComponent<Node>(entity);
    //     for (auto child : node.childs) {
    //         nodesToVisit.emplace(child, worldMat);
    //     }
    //     //}
    // }

    // bool hasActive = false;
    // for (auto& camera : entityRegistry().getAllComponents<Camera>()) {
    //     auto entity     = entityRegistry().getEntityFromComponent(camera);
    //     auto& transform = entityRegistry().getComponent<Transform>(entity);
    //     if (camera.active) {
    //         hasActive = true;
    //     }
    //     updateCamera(camera, transform.world);
    // }
    // if (!hasActive) {
    //     auto cameras = entityRegistry().getAllComponents<Camera>();
    //     if (!cameras.empty()) {
    //         cameras[0].active = true;
    //     }
    // }
}

// void SceneService::addDefaultPosition(Entity entity) {
//     if (!entityRegistry().hasComponent<Position>(entity)) {
//         auto& position    = entityRegistry().addComponent<Position>(entity);
//         position.rotation = { 0.0f, 0.0f, 0.0f, 1.0f };
//         position.scale    = { 1.0f, 1.0f, 1.0f };
//         position.updated  = true;
//     }
// }
//
// void SceneService::updateCamera(Camera& camera, const glm::mat4& world) {
//     auto pos = world[3].xyz();
//     auto dir = glm::transpose(world) * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
//
//     camera.fov      = std::clamp(camera.fov, glm::radians(30.0f), glm::radians(120.0f));
//     camera.position = pos;
//     camera.front    = glm::normalize(dir.xyz());
//     camera.right    = glm::normalize(glm::cross((glm::vec3&) camera.front, glm::vec3(0.0f, 1.0f, 0.0f)));
//     camera.up       = glm::normalize(glm::cross((glm::vec3&) camera.right, (glm::vec3&) camera.front));
//
//     gerium_uint16_t width, height;
//     gerium_application_get_size(application().handle(), &width, &height);
//
//     const auto aspect = float(width) / height;
//     const auto center = pos + (glm::vec3&) camera.front;
//
//     camera.prevViewProjection = camera.viewProjection;
//     camera.projection         = glm::perspective(camera.fov, aspect, camera.nearPlane, camera.farPlane);
//     camera.view               = glm::lookAt(pos, center, (glm::vec3&) camera.up);
//     camera.viewProjection     = (glm::mat4&) camera.projection * (glm::mat4&) camera.view;
// }
//
