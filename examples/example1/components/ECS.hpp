#ifndef ECS_HPP
#define ECS_HPP

#include "Name.hpp"

using Entity = uint32_t;

inline int componentIdSequence = 0;
template <typename T>
inline const int componentId = componentIdSequence++;

class ComponentPool {
public:
    virtual ~ComponentPool() = default;

    virtual bool remove(Entity entity) = 0;

    [[nodiscard]] virtual std::vector<char> serialize() const = 0;

    virtual void deserialize(const std::vector<char>& data) = 0;
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

    [[nodiscard]] const T& get(Entity entity) const noexcept {
        assert(_entityIndices.count(entity) && "Component not found");
        return _pool[_entityIndices.at(entity)];
    }

    [[nodiscard]] std::span<T> getAll() noexcept {
        return std::span{ _pool.data(), _pool.size() };
    }

    [[nodiscard]] std::span<const T> getAll() const noexcept {
        return std::span{ _pool.data(), _pool.size() };
    }

    [[nodiscard]] std::vector<char> serialize() const override {
        // Data data{ _pool };
        // data.map.reserve(_entityIndices.size());
        // for (const auto& [entity, index] : _entityIndices) {
        //     data.map.emplace_back(entity, (uint32_t) index);
        // }

        // return rfl::capnproto::write(data);
        return {};
    }

    void deserialize(const std::vector<char>& data) override {
        // _pool.clear();
        // _entityIndices.clear();
        // _indexEntities.clear();

        // auto components = rfl::capnproto::read<Data>(data).value();

        // _pool = std::move(components.pool);
        // for (const auto& entry : components.map) {
        //     _entityIndices[entry.entity] = entry.index;
        //     _indexEntities[entry.index]  = entry.entity;
        // }
    }

private:
    struct Entry {
        Entity entity;
        uint32_t index;
    };

    struct Data {
        std::vector<T> pool;
        std::vector<Entry> map;
    };

    std::vector<T> _pool{};
    std::map<Entity, size_t> _entityIndices{};
    std::map<size_t, Entity> _indexEntities{};
};

class ComponentManager final {
public:
    struct Data {
        std::vector<std::vector<char>> pools;
        std::vector<uint64_t> bits;
    };

    template <typename... Args>
    void registerTypes() {
        (getPool<Args>(), ...);
    }

    template <typename T>
    T& add(Entity entity) {
        bits(entity) |= (1 << componentId<T>);
        return getPool<T>()->add(entity);
    }

    template <typename T>
    bool remove(Entity entity) {
        bits(entity) &= ~(1 << componentId<T>);
        if (getPool<T>()->remove(entity)) {
            removeEmptyBits();
            return true;
        }
        return false;
    }

    void removeAll(Entity entity) {
        bits(entity) = 0;
        for (auto& [_, pool] : _pools) {
            pool->remove(entity);
        }
        removeEmptyBits();
    }

    template <typename T>
    [[nodiscard]] bool has(Entity entity) const noexcept {
        return bits(entity) & (1 << componentId<T>);
    }

    template <typename T>
    [[nodiscard]] T& get(Entity entity) noexcept {
        assert(has<T>(entity) && "Component not found");
        return getPool<T>()->get(entity);
    }

    template <typename T>
    [[nodiscard]] const T& get(Entity entity) const noexcept {
        assert(has<T>(entity) && "Component not found");
        return getPool<T>()->get(entity);
    }

    template <typename T>
    [[nodiscard]] std::span<T> getAll() noexcept {
        return getPool<T>()->getAll();
    }

    template <typename T>
    [[nodiscard]] std::span<const T> getAll() const noexcept {
        return getPool<T>()->getAll();
    }

    [[nodiscard]] Data serialize() const {
        Data data;
        for (auto& [_, pool] : _pools) {
            data.pools.push_back(std::move(pool->serialize()));
        }
        data.bits = _componentBits;
        return data;
    }

    void deserialize(Data&& data) {
        _componentBits.clear();
        for (int i = 0; i < data.pools.size(); ++i) {
            _pools[i]->deserialize(data.pools[i]);
        }
        _componentBits = std::move(data.bits);
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

    uint64_t& bits(Entity entity) const {
        if (entity >= _componentBits.size()) {
            _componentBits.resize(entity + 1);
        }
        return _componentBits[entity];
    }

    void removeEmptyBits() noexcept {
        while (!_componentBits.empty() && _componentBits.back() == 0) {
            _componentBits.pop_back();
        }
    }

    mutable std::map<int, std::unique_ptr<ComponentPool>> _pools;
    mutable std::vector<uint64_t> _componentBits;
};

class EntityManager final {
public:
    EntityManager() {
        registerComponent<Name>();
    }

    [[nodiscard]] Entity createEntity(const std::string& name = "") {
        if (!name.empty() && _mapEntities.contains(name)) {
            throw std::runtime_error("entity with name '" + name + "' already exists");
        }

        if (_entities.size() >= _freeEntities.size()) {
            _freeEntities.resize(_freeEntities.empty() ? 128 : _freeEntities.size() * 2);
            for (auto i = _entities.size(); i < _freeEntities.size(); ++i) {
                _freeEntities[i] = i;
            }
        }

        auto entity = _freeEntities[_entities.size()];
        _entities.insert(entity);

        if (!name.empty()) {
            addComponent<Name>(entity).name = name;
            _mapEntities[name]              = entity;
        }
        return entity;
    }

    void destroyEntity(Entity entity) {
        if (auto it = _entities.find(entity); it != _entities.end()) {
            _entities.erase(it);
        } else {
            throw std::invalid_argument("Out of range");
        }
        if (hasComponent<Name>(entity)) {
            const auto& name = getComponent<Name>(entity).name;
            _mapEntities.erase(name);
        }
        _componentManager.removeAll(entity);
        _freeEntities[_entities.size()] = entity;
    }

    [[nodiscard]] Entity getEntity(const std::string& name) const {
        if (auto it = _mapEntities.find(name); it != _mapEntities.end()) {
            return it->second;
        }
        throw std::runtime_error("entity with name '" + name + "' not found");
    }

    [[nodiscard]] const std::set<Entity>& entities() noexcept {
        return _entities;
    }

    template <typename T>
    void registerComponent() {
        _componentManager.registerTypes<T>();
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
    [[nodiscard]] bool hasComponent(Entity entity) const noexcept {
        return _componentManager.has<T>(entity);
    }

    template <typename T>
    [[nodiscard]] T& getComponent(Entity entity, bool addIfNotExist = false) {
        return hasComponent<T>(entity) ? _componentManager.get<T>(entity)
                                       : (addIfNotExist ? _componentManager.add<T>(entity)
                                                        : throw std::invalid_argument("Component not found"));
    }

    template <typename T>
    [[nodiscard]] const T& getComponent(Entity entity, bool addIfNotExist = false) const noexcept {
        auto manager = const_cast<ComponentManager*>(&_componentManager);
        return hasComponent<T>(entity)
                   ? _componentManager.get<T>(entity)
                   : (addIfNotExist ? manager->add<T>(entity) : throw std::invalid_argument("Component not found"));
    }

    template <typename T>
    [[nodiscard]] std::span<T> getAllComponents() noexcept {
        return _componentManager.getAll<T>();
    }

    template <typename T>
    [[nodiscard]] std::span<const T> getAllComponents() const noexcept {
        return _componentManager.getAll<T>();
    }

    [[nodiscard]] std::vector<char> serialize() const {
        // Data data{ _freeEntities, _entities, _componentManager.serialize() };
        // return rfl::capnproto::write(data);
        return {};
    }

    void deserialize(const std::vector<char>& data) {
        // _freeEntities.clear();
        // _entities.clear();
        // _mapEntities.clear();
        // auto entities = rfl::capnproto::read<Data>(data).value();

        // _freeEntities = std::move(entities.freeEntities);
        // _entities     = std::move(entities.entities);
        // _componentManager.deserialize(std::move(entities.components.get()));

        // for (auto entity : _entities) {
        //     if (hasComponent<Name>(entity)) {
        //         const auto& name   = getComponent<Name>(entity).name;
        //         _mapEntities[name] = entity;
        //     }
        // }
    }

private:
    struct Data {
        std::vector<Entity> freeEntities{};
        std::set<Entity> entities{};
        rfl::Flatten<ComponentManager::Data> components{};
    };

    std::vector<Entity> _freeEntities{};
    std::set<Entity> _entities{};
    std::unordered_map<std::string, Entity> _mapEntities{};
    ComponentManager _componentManager{};
};

#endif
