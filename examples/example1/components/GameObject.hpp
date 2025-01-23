#ifndef GAME_OBJECT_HPP
#define GAME_OBJECT_HPP

#include "../Common.hpp"
#include "ECS.hpp"

class Application;

class GameObject final : NonMovable {
public:
    GameObject() = default;
    GameObject(std::string name, Application* application);

    ~GameObject();

    const std::string name() const noexcept {
        return _name;
    }

    template <typename C>
    C& addComponent() {
        return _entityManager.addComponent<C>(_entity);
    }

    template <typename C>
    C& getComponent(bool addIfNotExist = false) {
        return _entityManager.getComponent<C>(_entity, addIfNotExist);
    }

    template <typename C>
    bool hasComponent() const noexcept {
        return _entityManager.hasComponent<C>(_entity);
    }

private:
    std::string _name{};
    EntityManager& _entityManager;
    Entity _entity{};
};

#endif
