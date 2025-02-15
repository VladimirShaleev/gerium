#include "PhysicsService.hpp"
#include "../components/Collider.hpp"
#include "../components/LocalTransform.hpp"
#include "../components/Node.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Static.hpp"
#include "../components/WorldTransform.hpp"

using namespace entt::literals;

void PhysicsService::createBodies() {
    auto view = entityRegistry().view<RigidBody, Collider, WorldTransform>();

    for (auto entity : view) {
        auto& rigidBody      = view.get<RigidBody>(entity);
        auto& collider       = view.get<Collider>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);
        auto isDynamic       = !entityRegistry().any_of<::Static>(entity);

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 position;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(worldTransform.matrix, scale, rotation, position, skew, perspective);

        auto pos  = JPH::Vec3(position.x, position.y, position.z);
        auto quat = JPH::Quat(rotation.x, rotation.y, rotation.z, rotation.w);
        auto size = collider.size * scale;

        JPH::Ref<JPH::Shape> shape{};
        switch (collider.shape) {
            case Collider::Shape::Box:
                shape = new JPH::BoxShape(JPH::Vec3(size.x, size.y, size.z));
                break;
            case Collider::Shape::Sphere:
                shape = new JPH::SphereShape(glm::max(glm::max(size.x, size.y), size.z));
                break;
            case Collider::Shape::Capsule:
                break;
        }

        JPH::BodyCreationSettings settings(shape,
                                           pos,
                                           quat,
                                           isDynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static,
                                           isDynamic ? ObjectLayers::Dynamic : ObjectLayers::Static);

        rigidBody.body = _physicsSystem->GetBodyInterface().CreateBody(settings)->GetID();
        _physicsSystem->GetBodyInterface().AddBody(rigidBody.body, JPH::EActivation::Activate);

        _physicsSystem->GetBodyInterface().SetLinearVelocity(rigidBody.body, JPH::Vec3(0.0f, -5.0f, 0.0f));
    }

    _physicsSystem->OptimizeBroadPhase();
}

void PhysicsService::start() {
    JPH::RegisterDefaultAllocator();

    JPH::Factory::sInstance = new JPH::Factory();

    JPH::RegisterTypes();

    auto numThreads = std::thread::hardware_concurrency() - 1;

    _allocator                     = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    _jobSystem                     = std::make_unique<JPH::JobSystemThreadPool>(512, 8, numThreads);
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
    _allocator                     = nullptr;

    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
}

void PhysicsService::update(gerium_uint64_t /* elapsedMs */, gerium_float64_t elapsed) {
    _physicsSystem->Update(elapsed, 1, _allocator.get(), _jobSystem.get());

    syncPhysicsToECS();

    // auto&& physicsTransforms = entityRegistry().storage<WorldTransform>("physics_transforms"_hs);

    // updatePhysicsTransforms(physicsTransforms);
    // updateLocalTransforms(physicsTransforms);

    // physicsTransforms.clear();
}

void PhysicsService::syncPhysicsToECS() {
    auto view = entityRegistry().view<RigidBody, WorldTransform>(entt::exclude<::Static>);

    for (auto entity : view) {
        auto& rigidBody      = view.get<RigidBody>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);

        auto position = _physicsSystem->GetBodyInterface().GetCenterOfMassPosition(rigidBody.body);
        auto velocity = _physicsSystem->GetBodyInterface().GetLinearVelocity(rigidBody.body);

        auto newPos = glm::vec4(position.GetX(), position.GetY(), position.GetZ(), worldTransform.matrix[3][3]);

        worldTransform.prevMatrix = worldTransform.matrix;
        worldTransform.matrix[3]  = newPos;
    }
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
    return magic_enum::enum_count<ObjectLayers>();
}

JPH::BroadPhaseLayer PhysicsService::BroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const {
    switch (inLayer) {
        case ObjectLayers::Static:
            return JPH::BroadPhaseLayer(0);
        case ObjectLayers::Dynamic:
            return JPH::BroadPhaseLayer(1);
        case ObjectLayers::Trigger:
            return JPH::BroadPhaseLayer(2);
        default:
            return JPH::BroadPhaseLayer(0);
    }
}

PhysicsService::ObjectVsBroadPhaseLayerFilter::ObjectVsBroadPhaseLayerFilter(PhysicsService* service) noexcept :
    _service(service) {
}

bool PhysicsService::ObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1,
                                                                  JPH::BroadPhaseLayer inLayer2) const {
    switch (inLayer1) {
        case Static:
            return inLayer2 == JPH::BroadPhaseLayer(Dynamic);
        case Dynamic:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
    }
}

PhysicsService::ObjectLayerPairFilter::ObjectLayerPairFilter(PhysicsService* service) noexcept : _service(service) {
}

bool PhysicsService::ObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const {
    return true;
}
