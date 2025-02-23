#ifndef SERVICES_PHYSICS_SERVICE_HPP
#define SERVICES_PHYSICS_SERVICE_HPP

#include "../Model.hpp"
#include "../components/Collider.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Transform.hpp"
#include "../components/Vehicle.hpp"
#include "../components/Wheel.hpp"
#include "ServiceManager.hpp"

class PhysicsService final : public Service {
public:
    void createBodies(const Cluster& cluster);

private:
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;
    void step();

    void createVehicleConstraints(entt::entity entity, Vehicle& vehicle, RigidBody& rigidBody);

    entt::hashed_string stateName() const noexcept override;
    std::vector<gerium_uint8_t> saveState() override;
    void restoreState(const std::vector<gerium_uint8_t>& data) override;

    void driverInput(entt::entity entity, Vehicle& vehicle);
    void updateVehicleSettings(const Vehicle& vehicle, JPH::Ref<JPH::VehicleConstraint>& constraint);
    void syncPhysicsToECS();

    static JPH::WheelSettings* createWheelSettings(const Vehicle& vehicle,
                                                   const Wheel& wheel,
                                                   gerium_float32_t radius,
                                                   gerium_float32_t width);

    static JPH::Ref<JPH::Shape> getShape(const Collider& collider, const Cluster& cluster);

    enum ObjectLayers : JPH::uint8 {
        Static  = 0,
        Dynamic = 1
    };

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

    std::unique_ptr<JPH::TempAllocator> _allocator{};
    std::unique_ptr<JPH::JobSystem> _jobSystem{};
    std::unique_ptr<JPH::BroadPhaseLayerInterface> _broadPhaseLayerInterface{};
    std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilter> _objectVsBroadPhaseLayerFilter{};
    std::unique_ptr<JPH::ObjectLayerPairFilter> _objectLayerPairFilter{};
    std::unique_ptr<JPH::PhysicsSystem> _physicsSystem{};
    JPH::Ref<JPH::VehicleCollisionTester> _vehicleTester{};
    std::map<entt::entity, JPH::Ref<JPH::VehicleConstraint>> _vehicleConstraints{};
};

#endif
