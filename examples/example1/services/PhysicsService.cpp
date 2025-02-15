#include "PhysicsService.hpp"
#include "../components/LocalTransform.hpp"
#include "../components/Node.hpp"
#include "../components/RigidBody.hpp"
#include "../components/WorldTransform.hpp"

using namespace entt::literals;

void PhysicsService::start() {
    JPH::RegisterDefaultAllocator();

    _broadPhaseLayerInterface      = std::make_unique<BroadPhaseLayerInterface>(this);
    _objectVsBroadPhaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilter>(this);
    _objectLayerPairFilter         = std::make_unique<ObjectLayerPairFilter>(this);

    _physicsSystem = std::make_unique<JPH::PhysicsSystem>();
    _physicsSystem->Init(MAX_INSTANCES_PER_TECHNIQUE,
                         0,
                         MAX_INSTANCES_PER_TECHNIQUE,
                         MAX_INSTANCES_PER_TECHNIQUE,
                         *_broadPhaseLayerInterface.get(),
                         *_objectVsBroadPhaseLayerFilter.get(),
                         *_objectLayerPairFilter.get());
}

void PhysicsService::stop() {
    _physicsSystem                 = nullptr;
    _objectLayerPairFilter         = nullptr;
    _objectVsBroadPhaseLayerFilter = nullptr;
    _broadPhaseLayerInterface      = nullptr;
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
        const auto node = entityRegistry().try_get<Node>(entity);

        const auto parentInverseWorldMatrix =
            node && node->parent != entt::null ? glm::inverse(entityRegistry().get<WorldTransform>(node->parent).matrix)
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

PhysicsService::BroadPhaseLayerInterface::BroadPhaseLayerInterface(PhysicsService* service) noexcept :
    _service(service) {
}

JPH::uint PhysicsService::BroadPhaseLayerInterface::GetNumBroadPhaseLayers() const {
    return 2;
}

JPH::BroadPhaseLayer PhysicsService::BroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
    return JPH::BroadPhaseLayer(inLayer < JPH::uint16(2) ? JPH::uint8(inLayer) : JPH::uint8(0));
}

PhysicsService::ObjectVsBroadPhaseLayerFilter::ObjectVsBroadPhaseLayerFilter(PhysicsService* service) noexcept :
    _service(service) {
}

bool PhysicsService::ObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1,
                                                                  JPH::BroadPhaseLayer inLayer2) const {
    return true;
}

PhysicsService::ObjectLayerPairFilter::ObjectLayerPairFilter(PhysicsService* service) noexcept : _service(service) {
}

bool PhysicsService::ObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const {
    return true;
}
