#ifndef ENTITY_COMPONENT_SYSTEM_HPP
#define ENTITY_COMPONENT_SYSTEM_HPP

#include "Common.hpp"

static constexpr gerium_uint16_t NoneIndex = std::numeric_limits<gerium_uint16_t>::max();

class Entity {
public:
    Entity() = default;

    Entity(const Entity&)            = delete;
    Entity& operator=(const Entity&) = delete;

    gerium_uint16_t componentCount() const noexcept {
        return _count;
    }

private:
    template <typename... Components>
    friend class Registry;

    gerium_uint16_t _count{};
    gerium_uint16_t _index{ NoneIndex };
    gerium_uint16_t _pool{};
};

class Component {
public:
    virtual ~Component()  = default;
    virtual void update(Entity& entity, gerium_data_t data) = 0;

private:
    template <typename C>
    friend class ComponentPoolImpl;

    template <typename... Components>
    friend class Registry;

    gerium_uint16_t _index{ NoneIndex };
    gerium_uint16_t _next{ NoneIndex };
    gerium_uint16_t _nextPool{};
};

class ComponentPool {
public:
    virtual ~ComponentPool() = default;

    virtual void release(gerium_uint16_t index) noexcept      = 0;
    virtual Component* access(gerium_uint16_t index) noexcept = 0;
};

template <typename C>
class ComponentPoolImpl final : public ComponentPool {
public:
    static_assert(std::is_base_of_v<Component, C>, "C must inheritance from Component");

    ComponentPoolImpl(gerium_uint16_t poolSize = 128) :
        _poolSize(0),
        _head(0),
        _indices(nullptr),
        _components(nullptr) {
        reserve(poolSize);
    }

    ~ComponentPoolImpl() {
        clear();
    }

    ComponentPoolImpl(const ComponentPoolImpl&) = delete;

    ComponentPoolImpl& operator=(const ComponentPoolImpl&) = delete;

    ComponentPoolImpl(ComponentPoolImpl&& other) noexcept :
        _poolSize(other._poolSize),
        _head(other._head),
        _indices(other._indices),
        _components(other._components) {
        other._poolSize   = 0;
        other._head       = 0;
        other.other       = nullptr;
        other._components = nullptr;
    }

    ComponentPoolImpl& operator=(ComponentPoolImpl&& other) noexcept {
        if (this != &other) {
            clear();
            _poolSize         = other._poolSize;
            _head             = other._head;
            _indices          = other._indices;
            _components       = other._components;
            other._poolSize   = 0;
            other._head       = 0;
            other._indices    = nullptr;
            other._components = nullptr;
        }
        return *this;
    }

    gerium_uint16_t size() const noexcept {
        return _head;
    }

    void reserve(gerium_uint16_t newPoolSize) {
        assert(newPoolSize > _poolSize);
        auto indices = reinterpret_cast<gerium_uint16_t*>(realloc(_indices, newPoolSize * sizeof(gerium_uint16_t)));
        if (!indices) {
            throw std::bad_alloc();
        }

        auto components = reinterpret_cast<C*>(realloc(_components, newPoolSize * sizeof(C)));
        memset((void*) (components + _poolSize), 0, (newPoolSize - _poolSize) * sizeof(C));

        if (!components) {
            free(indices);
            throw std::bad_alloc();
        }

        for (gerium_uint16_t i = _poolSize; i < newPoolSize; ++i) {
            indices[i] = i;
        }

        _poolSize   = newPoolSize;
        _indices    = indices;
        _components = components;
    }

    void clear() noexcept {
        for (gerium_uint16_t i = 0; i < _head; ++i) {
            static_cast<C*>(access(i))->~C();
        }

        _head     = 0;
        _poolSize = 0;

        if (_components) {
            free(_components);
            _components = nullptr;
        }

        if (_indices) {
            free(_indices);
            _indices = nullptr;
        }
    }

    gerium_uint16_t obtain() {
        assert(_head < std::numeric_limits<gerium_uint16_t>::max());

        if (_head >= _poolSize) {
            reserve(_poolSize << 1);
        }

        const auto index = _indices[_head++];
        auto component   = new (access(index)) C();

        return { index };
    }

    void release(gerium_uint16_t index) noexcept override {
        access(index)->~Component();
        std::memset(&_components[index], 0, sizeof(C));

        _indices[--_head] = index;
    }

    Component* access(gerium_uint16_t index) noexcept override {
        return &_components[index];
    }

    std::pair<gerium_uint16_t, C*> obtainAndAccess() {
        const auto handle = obtain();
        return { handle, static_cast<C*>(access(handle)) };
    }

    void getComponents(gerium_uint16_t& count, C** results) noexcept {
        gerium_uint16_t result = 0;
        for (gerium_uint16_t i = 0; i < _head && result < count; ++i) {
            if (_components[i]._index != NoneIndex) {
                results[result] = &_components[i];
                ++result;
            }
        }
        count = result;
    }

private:
    gerium_uint16_t _poolSize;
    gerium_uint16_t _head;
    gerium_uint16_t* _indices;
    C* _components;
};

template <typename...>
struct index;

template <typename T, typename... R>
struct index<T, T, R...> : std::integral_constant<size_t, 0> {};

template <typename T, typename F, typename... R>
struct index<T, F, R...> : std::integral_constant<size_t, 1 + index<T, R...>::value> {};

template <typename... Components>
class Registry {
public:
    constexpr Registry() {
        fillPoolRefs(std::make_index_sequence<sizeof...(Components)>{});
    }

    ~Registry() {
        clear();
    }

    void removeEntity(Entity& entity) {
        if (entity._index == NoneIndex) {
            return;
        }
        auto currentIndex = entity._index;
        auto currentPool  = entity._pool;
        auto current      = _poolRefs[currentPool]->access(currentIndex);
        while (currentIndex != NoneIndex) {
            auto nextIndex = current->_next;
            auto nextPool  = current->_nextPool;
            _poolRefs[currentPool]->release(currentIndex);

            currentIndex = nextIndex;
            currentPool  = nextPool;
            current      = _poolRefs[currentPool]->access(currentIndex);
        }
        entity._count = 0;
        entity._index = NoneIndex;
        entity._pool  = 0;
    }

    template <typename C>
    C* addComponent(Entity& entity, const C& component) {
        static_assert(std::is_base_of_v<Component, C>, "C must inheritance from Component");

        auto& pool     = getPool<C>();
        auto poolIndex = getPoolIndex<C>();

        const auto [index, c] = pool.obtainAndAccess();
        *c                    = component;
        c->_index             = index;

        if (auto lastComponent = getLastComponent(entity)) {
            lastComponent->_next     = index;
            lastComponent->_nextPool = poolIndex;
        } else {
            entity._index = index;
            entity._pool  = poolIndex;
        }
        ++entity._count;

        return c;
    }

    template <typename C>
    C* getComponent(Entity& entity) {
        if (entity._index == NoneIndex) {
            return nullptr;
        }
        auto findPool    = getPoolIndex<C>();
        auto currentPool = entity._pool;

        auto current = _poolRefs[entity._pool]->access(entity._index);
        while (true) {
            if (currentPool == findPool) {
                return static_cast<C*>(current);
            }
            if (current->_next == NoneIndex) {
                break;
            }
            currentPool = current->_nextPool;
            current     = _poolRefs[current->_nextPool]->access(current->_next);
        }
        return nullptr;
    }

    void getComponents(Entity& entity, gerium_uint16_t& count, Component** results) noexcept {
        count = std::min(entity.componentCount(), count);

        auto current = _poolRefs[entity._pool]->access(entity._index);

        for (gerium_uint16_t i = 0; i < count; ++i) {
            results[i] = current;
            current    = _poolRefs[current->_nextPool]->access(current->_next);
        }
    }

    template <typename C>
    void getComponents(gerium_uint16_t& count, C** results) noexcept {
        getPool<C>().getComponents(count, results);
    }

    void update(Entity& entity, gerium_data_t data) {
        if (entity._index == NoneIndex) {
            return;
        }
        auto current = _poolRefs[entity._pool]->access(entity._index);
        current->update(entity, data);
        while (current->_next != NoneIndex) {
            current = _poolRefs[current->_nextPool]->access(current->_next);
            current->update(entity, data);
        }
    }

    void clear() noexcept {
        (std::get<ComponentPoolImpl<Components>>(_pools).clear(), ...);
    }

private:
    template <typename C>
    constexpr ComponentPoolImpl<C>& getPool() noexcept {
        static_assert(std::is_base_of_v<Component, C>, "C must inheritance from Component");
        return std::get<ComponentPoolImpl<C>>(_pools);
    }

    template <typename C>
    constexpr gerium_uint16_t getPoolIndex() const noexcept {
        static_assert(std::is_base_of_v<Component, C>, "C must inheritance from Component");
        return index<C, Components...>::value;
    }

    constexpr Component* getLastComponent(const Entity& entity) noexcept {
        if (entity._index == NoneIndex) {
            return nullptr;
        }

        auto current = _poolRefs[entity._pool]->access(entity._index);
        while (current->_next != NoneIndex) {
            current = _poolRefs[current->_nextPool]->access(current->_next);
        }
        return current;
    }

    template <std::size_t... I>
    constexpr void fillPoolRefs(std::index_sequence<I...>) {
        ((_poolRefs[I] = &std::get<I>(_pools)), ...);
    }

    std::tuple<ComponentPoolImpl<Components>...> _pools{};
    std::array<ComponentPool*, sizeof...(Components)> _poolRefs{};
};

#endif
