#ifndef SCENE_SERVICE_HPP
#define SCENE_SERVICE_HPP

#include "../Model.hpp"
#include "../ResourceManager.hpp"
#include "../components/Name.hpp"
#include "../events/AddModelEvent.hpp"
#include "../events/DeleteNodeEvent.hpp"
#include "ServiceManager.hpp"

class SceneService final : public Service {
private:
    static constexpr entt::hashed_string ROOT = { "" };

    void onAddModel(const AddModelEvent& event);
    void onDeleteNode(const DeleteNodeEvent& event);

    void checkAndAddNode(entt::entity entity, const Name& name);
    void reloadCluster();
    const Model& getModel(const entt::hashed_string& modelId);

    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    entt::hashed_string stateName() const noexcept override;
    std::vector<gerium_uint8_t> saveState() override;
    void restoreState(const std::vector<gerium_uint8_t>& data) override;
    
    std::map<entt::hashed_string, entt::entity> _nodes{};
    std::map<entt::hashed_string, Model> _models{};
    Cluster _cluster{};
    bool _clusterFlushed{};
};

#endif
