#ifndef GAME_SERVICE_HPP
#define GAME_SERVICE_HPP

#include "ServiceManager.hpp"

class GameService : public Service {
public:
    // void addModel(const std::string& name, const std::string& filename, const std::string& parent = "");
    // void addCamera(const std::string& name, const std::string& parent = "");
    // void activeCamera(const std::string& name);

protected:
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

private:
    // Entity getParentEntity(const std::string& parent);
};

#endif
