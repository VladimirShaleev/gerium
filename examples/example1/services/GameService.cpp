#include "GameService.hpp"
#include "../Application.hpp"
#include "RenderService.hpp"
#include "SceneService.hpp"

// void GameService::addModel(const std::string& name, const std::string& filename, const std::string& parent) {
//     auto model = manager().getService<RenderService>()->resourceManager().loadModel(filename);
//     manager().getService<SceneService>()->addModel(getParentEntity(parent), name, model);
// }
//
// void GameService::addCamera(const std::string& name, const std::string& parent) {
//     auto entity  = entityRegistry().createEntity(name);
//     auto& camera = entityRegistry().addComponent<Camera>(entity);
//     manager().getService<SceneService>()->addChild(getParentEntity(parent), entity);
//
//     camera.movementSpeed = 0.001f;
//     camera.rotationSpeed = 0.001f;
//     camera.nearPlane     = 0.01f;
//     camera.farPlane      = 1000.0f;
//     camera.fov           = glm::radians(60.0f);
//
//     entityRegistry().getComponent<Position>(entity).rotation =
//         glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
// }
//
// void GameService::activeCamera(const std::string& name) {
//     for (auto& camera : entityRegistry().getAllComponents<Camera>()) {
//         camera.active = false;
//     }
//     entityRegistry().getComponent<Camera>(entityRegistry().getEntity(name)).active = true;
// }

void GameService::start() {
}

void GameService::stop() {
}

void GameService::update(gerium_uint64_t /* elapsedMs */, gerium_float64_t /* elapsed */) {
}

// Entity GameService::getParentEntity(const std::string& parent) {
//     return parent.empty() ? manager().getService<SceneService>()->root() : entityRegistry().getEntity(parent);
// }
