#include "ClearService.hpp"
#include "../components/Collider.hpp"
#include "../components/Renderable.hpp"
#include "../components/RigidBody.hpp"
#include "../components/Transform.hpp"

void ClearService::update(gerium_uint64_t /* elapsedMs */, gerium_float64_t /* elapsed */) {
    auto& chs = changes();
    clearChanges<Transform>(chs.transforms);
    clearChanges<RigidBody>(chs.rigidBodies);
    clearChanges<Collider>(chs.colliders);
    clearChanges<Renderable>(chs.renderables);
}
