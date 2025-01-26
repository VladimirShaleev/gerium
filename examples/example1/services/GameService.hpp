#ifndef GAME_SERVICE_HPP
#define GAME_SERVICE_HPP

#include "../components/ECS.hpp"
#include "ServiceManager.hpp"

class GameService : public Service {
public:

protected:
    void start() override;
    void stop() override;
    void update() override;

private:
};

#endif
