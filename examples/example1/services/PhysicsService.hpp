#ifndef SERVICES_PHYSICS_SERVICE_HPP
#define SERVICES_PHYSICS_SERVICE_HPP

#include "../components/WorldTransform.hpp"
#include "ServiceManager.hpp"

class PhysicsService final : public Service {
private:
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    void updatePhysicsTransforms(entt::storage<WorldTransform>& storage);
    void updateLocalTransforms(entt::storage<WorldTransform>& storage);

    glm::mat4 getPhysicsTransform();

    class BroadPhaseLayerInterface : public JPH::BroadPhaseLayerInterface {
    public:
        explicit BroadPhaseLayerInterface(PhysicsService* service) noexcept;

        JPH::uint GetNumBroadPhaseLayers() const override;

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;

        PhysicsService* _service;
    };

    class ObjectVsBroadPhaseLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        explicit ObjectVsBroadPhaseLayerFilter(PhysicsService* service) noexcept;

        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;

        PhysicsService* _service;
    };

    class ObjectLayerPairFilter : public JPH::ObjectLayerPairFilter {
    public:
        explicit ObjectLayerPairFilter(PhysicsService* service) noexcept;

        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::ObjectLayer inLayer2) const override;

        PhysicsService* _service;
    };

    std::unique_ptr<BroadPhaseLayerInterface> _broadPhaseLayerInterface{};
    std::unique_ptr<ObjectVsBroadPhaseLayerFilter> _objectVsBroadPhaseLayerFilter{};
    std::unique_ptr<ObjectLayerPairFilter> _objectLayerPairFilter{};
    std::unique_ptr<JPH::PhysicsSystem> _physicsSystem{};
};

#endif
