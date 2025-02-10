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
};

#endif
