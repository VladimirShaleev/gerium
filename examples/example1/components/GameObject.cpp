#include "GameObject.hpp"
#include "../Application.hpp"

GameObject::GameObject(std::string name, Application* application) :
    _name(std::move(name)),
    _entityManager(application->entityManager()),
    _entity(application->entityManager().createEntity()) {
}

GameObject::~GameObject() {
    _entityManager.destroyEntity(_entity);
}
