#ifndef SCENE_SERVICE_HPP
#define SCENE_SERVICE_HPP

#include "../Model.hpp"
#include "../ResourceManager.hpp"
#include "ServiceManager.hpp"

class SceneService final : public Service {
public:
    // Entity root();
    // Entity addModel(Entity parent, const std::string& name, const Model* model);
    // void addChild(Entity parent, Entity child);
    // void removeChild(Entity child);
    // void clear();

private:
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    // void addDefaultPosition(Entity entity);

    // void updateCamera(Camera& camera, const glm::mat4& world);

    // Entity _root{ UndefinedNode };
};

#endif
