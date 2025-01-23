#ifndef ECS_HPP
#define ECS_HPP

#include "../Common.hpp"

using Entity = uint32_t;

inline int componentIdSequence = 0;
template <typename T>
inline const int componentId = componentIdSequence++;

class ComponentPool {
public:
    virtual ~ComponentPool() = default;

    virtual bool remove(Entity entity) = 0;
};

template <typename T>
class ComponentPoolImpl final : public ComponentPool {
public:
    T& add(Entity entity) {
        assert(!_entityIndices.count(entity) && "The component has already been added");
        auto index             = _pool.size();
        _entityIndices[entity] = index;
        _indexEntities[index]  = entity;
        _pool.push_back({});
        return _pool.back();
    }

    bool remove(Entity entity) override {
        if (_entityIndices.count(entity)) {
            auto removeIndex   = _entityIndices[entity];
            auto lastIndex     = _pool.size() - 1;
            _pool[removeIndex] = _pool[lastIndex];

            auto lastEntity             = _indexEntities[lastIndex];
            _entityIndices[lastEntity]  = removeIndex;
            _indexEntities[removeIndex] = lastEntity;

            _entityIndices.erase(entity);
            _indexEntities.erase(lastEntity);
            _pool.pop_back();
            return true;
        }
        return false;
    }

    T& get(Entity entity) noexcept {
        assert(_entityIndices.count(entity) && "Component not found");
        return _pool[_entityIndices[entity]];
    }

    const T& get(Entity entity) const noexcept {
        assert(_entityIndices.count(entity) && "Component not found");
        return _pool[_entityIndices[entity]];
    }

    std::span<T> getAll() noexcept {
        return std::span{ _pool.data(), _pool.size() };
    }

    std::span<const T> getAll() const noexcept {
        return std::span{ _pool.data(), _pool.size() };
    }

private:
    std::vector<T> _pool{};
    std::map<Entity, size_t> _entityIndices{};
    std::map<size_t, Entity> _indexEntities{};
};

template <size_t MaxEntities>
class ComponentManager final {
public:
    template <typename T>
    T& add(Entity entity) {
        _componentBits[entity] |= (1 << componentId<T>);
        return getPool<T>()->add(entity);
    }

    template <typename T>
    bool remove(Entity entity) {
        _componentBits[entity] &= ~(1 << componentId<T>);
        return getPool<T>()->remove(entity);
    }

    void removeAll(Entity entity) {
        _componentBits[entity] = 0;
        for (auto& [_, pool] : _pools) {
            pool->remove(entity);
        }
    }

    template <typename T>
    bool has(Entity entity) const noexcept {
        return _componentBits[entity] & (1 << componentId<T>);
    }

    template <typename T>
    T& get(Entity entity) noexcept {
        assert(has<T>(entity) && "Component not found");
        return getPool<T>()->get(entity);
    }

    template <typename T>
    const T& get(Entity entity) const noexcept {
        assert(has<T>(entity) && "Component not found");
        return getPool<T>()->get(entity);
    }

    template <typename T>
    std::span<T> getAll() noexcept {
        return getPool<T>()->getAll();
    }

    template <typename T>
    std::span<const T> getAll() const noexcept {
        return getPool<T>()->getAll();
    }

private:
    template <typename T>
    ComponentPoolImpl<T>* getPool() const {
        auto id = componentId<T>;

        if (auto it = _pools.find(id); it != _pools.end()) {
            return (ComponentPoolImpl<T>*) it->second.get();
        }

        auto pool   = std::make_unique<ComponentPoolImpl<T>>();
        auto result = pool.get();

        _pools[id] = std::move(pool);
        return result;
    }

    mutable std::map<int, std::unique_ptr<ComponentPool>> _pools;
    std::array<uint64_t, MaxEntities> _componentBits;
};

class EntityManager final {
public:
    static constexpr size_t MaxEntities = 512;

    EntityManager() {
        for (Entity entity = 0; entity < MaxEntities; ++entity) {
            _freeEntities[entity] = entity;
        }
    }

    [[nodiscard]] Entity createEntity() {
        if (_entities.size() >= MaxEntities) {
            throw std::bad_alloc();
        }

        auto entity = _freeEntities[_entities.size()];
        _entities.insert(entity);
        return entity;
    }

    void destroyEntity(Entity entity) {
        if (auto it = _entities.find(entity); it != _entities.end()) {
            _entities.erase(it);
        } else {
            throw std::invalid_argument("Out of range");
        }
        _componentManager.removeAll(entity);
        _freeEntities[_entities.size()] = entity;
    }

    const std::set<Entity>& entities() noexcept {
        return _entities;
    }

    template <typename T>
    T& addComponent(Entity entity) {
        return _componentManager.add<T>(entity);
    }

    template <typename T>
    bool removeComponent(Entity entity) {
        return _componentManager.remove<T>(entity);
    }

    template <typename T>
    bool hasComponent(Entity entity) const noexcept {
        return _componentManager.has<T>(entity);
    }

    template <typename T>
    T& getComponent(Entity entity, bool addIfNotExist = false) {
        return hasComponent<T>(entity) ? _componentManager.get<T>(entity)
                                       : (addIfNotExist ? _componentManager.add<T>(entity)
                                                        : throw std::invalid_argument("Component not found"));
    }

    template <typename T>
    const T& getComponent(Entity entity, bool addIfNotExist = false) const noexcept {
        auto manager = const_cast<ComponentManager<MaxEntities>*>(&_componentManager);
        return hasComponent<T>(entity)
                   ? _componentManager.get<T>(entity)
                   : (addIfNotExist ? manager->add<T>(entity) : throw std::invalid_argument("Component not found"));
    }

    template <typename T>
    std::span<T> getAllComponents() noexcept {
        return _componentManager.getAll<T>();
    }

    template <typename T>
    std::span<const T> getAllComponents() const noexcept {
        return _componentManager.getAll<T>();
    }

private:
    std::array<Entity, MaxEntities> _freeEntities{};
    std::set<Entity> _entities{};
    ComponentManager<MaxEntities> _componentManager{};
};

#endif
