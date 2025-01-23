#ifndef GAME_SERVICE_HPP
#define GAME_SERVICE_HPP

#include "../components/GameObject.hpp"
#include "ServiceManager.hpp"

class GameService : public Service {
public:
    GameObject* createObject(std::string name);

protected:
    void start() override;
    void stop() override;
    void update() override;

private:
    std::unordered_map<std::string, std::unique_ptr<GameObject>> _objects{};
};

#endif
