#include "GameService.hpp"
#include "../Application.hpp"
#include "../components/Position.hpp"

GameObject* GameService::createObject(std::string name) {
    if (_objects.contains(name)) {
        throw std::runtime_error("GameObject with name \"" + name + "\" already exists");
    }
    auto obj       = std::make_unique<GameObject>(std::move(name), &application());
    auto result    = obj.get();
    _objects[name] = std::move(obj);
    return result;
}

void GameService::start() {
    auto obj = createObject("test");

    obj->addComponent<Position>();

    auto& pos = obj->getComponent<Position>();

    pos.x = 2.0f;
    pos.y = 3.0f;
    pos.w = 1.0f;

    const std::string result = rfl::json::write(pos);
    const std::vector<char> bytes = rfl::capnproto::write(pos);

    auto test = rfl::capnproto::read<Position>(bytes).value();

    gerium_logger_print(application().logger(), GERIUM_LOGGER_LEVEL_INFO, result.c_str());
}

void GameService::stop() {
}

void GameService::update() {
}
