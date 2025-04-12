#ifndef SCENE_SERVICE_HPP
#define SCENE_SERVICE_HPP

#include "../Model.hpp"
#include "../ResourceManager.hpp"
#include "../components/Name.hpp"
#include "../events/AddModelEvent.hpp"
#include "../events/AddNodeNameEvent.hpp"
#include "../events/ChangeColliderEvent.hpp"
#include "../events/ChangeMaterialsEvent.hpp"
#include "../events/ChangeNodeNameEvent.hpp"
#include "../events/ChangeRigidBodyEvent.hpp"
#include "../events/DeleteNodeByNameEvent.hpp"
#include "../events/DeleteNodeEvent.hpp"
#include "../events/MoveNodeEvent.hpp"
#include "ServiceManager.hpp"

class SceneService final : public Service {
private:
    struct InvParentTransform {
        glm::mat4 invParentMatrix;
        glm::vec3 invParentScale;
    };

    static constexpr entt::hashed_string ROOT = { "" };

    void onAddModel(const AddModelEvent& event);
    void onAddNodeName(const AddNodeNameEvent& event);
    void onChangeNodeName(const ChangeNodeNameEvent& event);
    void onDeleteNode(const DeleteNodeEvent& event);
    void onDeleteNodeByName(const DeleteNodeByNameEvent& event);
    void onMoveNode(const MoveNodeEvent& event);
    void onChangeCollider(const ChangeColliderEvent& event);
    void onChangeRigidBody(const ChangeRigidBodyEvent& event);
    void onChangeMaterials(const ChangeMaterialsEvent& event);

    void checkAndAddNode(entt::entity entity, const Name& name);
    const Model& getModel(const entt::hashed_string& modelId);

    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    std::map<entt::hashed_string, entt::entity> _nodes{};
    std::map<entt::hashed_string, Model> _models{};
};

#endif
