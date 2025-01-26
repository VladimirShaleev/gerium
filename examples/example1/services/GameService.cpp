#include "GameService.hpp"
#include "../Application.hpp"
#include "../components/Position.hpp"

void GameService::start() {
    auto obj1 = entityManager().createEntity("test1");
    auto obj2 = entityManager().createEntity("test2");

    auto& pos = entityManager().addComponent<Position>(obj2);
    pos.x     = 2.0f;
    pos.y     = 3.0f;
    pos.z     = 0.0f;
    pos.w     = 1.0f;

    auto data = entityManager().serialize();

    entityManager().destroyEntity(obj2);
    entityManager().destroyEntity(obj1);

    entityManager().deserialize(data);

    obj1 = entityManager().getEntity("test1");
    obj2 = entityManager().getEntity("test2");

    auto& p1 = entityManager().getComponent<Position>(obj2);
    auto a   = p1;
}

void GameService::stop() {
}

void GameService::update() {
}
