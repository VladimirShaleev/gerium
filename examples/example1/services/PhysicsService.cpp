#include "PhysicsService.hpp"
#include "../components/Children.hpp"
#include "../components/LocalTransform.hpp"
#include "../components/Parent.hpp"
#include "../components/RigidBody.hpp"
#include "../components/WorldTransform.hpp"

using namespace entt::literals;

void PhysicsService::start() {
}

void PhysicsService::stop() {
}

void PhysicsService::update(gerium_uint64_t /* elapsedMs */, gerium_float64_t /* elapsed */) {
    auto&& physicsTransforms = entityRegistry().storage<WorldTransform>("physics_transforms"_hs);

    updatePhysicsTransforms(physicsTransforms);
    updateLocalTransforms(physicsTransforms);

    physicsTransforms.clear();
}

void PhysicsService::updatePhysicsTransforms(entt::storage<WorldTransform>& storage) {
    auto view = entityRegistry().view<RigidBody, WorldTransform>();

    for (auto entity : view) {
        auto& rigidBody      = view.get<RigidBody>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);

        if (!rigidBody.isKinematic) {
            auto physicsMatrix = worldTransform.matrix; // getPhysicsTransform();

            if (worldTransform.matrix != physicsMatrix) {
                worldTransform.matrix = physicsMatrix;
                storage.push(entity);
            }
        }
    }
}

void PhysicsService::updateLocalTransforms(entt::storage<WorldTransform>& storage) {
    auto view = entityRegistry().view<LocalTransform>() | entt::basic_view{ storage };

    for (auto entity : view) {
        const auto& parent = entityRegistry().try_get<Parent>(entity);

        const auto parentInverseWorldMatrix =
            parent ? glm::inverse(entityRegistry().get<WorldTransform>(parent->parent).matrix)
                   : glm::identity<glm::mat4>();

        const auto& worldMatrix = view.get<WorldTransform>(entity).matrix;
        auto localMatrix        = parentInverseWorldMatrix * worldMatrix;

        auto& localTransform = view.get<LocalTransform>(entity);
        assert(!localTransform.isDirty && "The physics service can only work with calculated world transforms.");

        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(
            localMatrix, localTransform.scale, localTransform.rotation, localTransform.position, skew, perspective);
        localTransform.isDirty = true;
    }
}

glm::mat4 PhysicsService::getPhysicsTransform() {
    return {};
}
