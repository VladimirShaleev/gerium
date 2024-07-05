#ifndef GERIUM_RESOURCE_POOL_HPP
#define GERIUM_RESOURCE_POOL_HPP

#include "Gerium.hpp"

namespace gerium {

struct Handle {
    gerium_uint16_t index;

    bool operator==(const Handle& rhs) const noexcept {
        return index == rhs.index;
    }

    bool operator!=(const Handle& rhs) const noexcept {
        return !(*this == rhs);
    }

    auto operator<=>(const Handle&) const = default;

    template <typename H>
    operator H() const noexcept {
        static_assert(sizeof(H) == sizeof(Handle));
        H result;
        memcpy(&result, &index, sizeof(H));
        return result;
    }
};

constexpr Handle Undefined = Handle{ Handle{ static_cast<uint16_t>(-1) } };

template <typename T, typename Resource>
class ResourcePoolIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type*;
    using const_pointer     = const value_type*;
    using const_reference   = const value_type*;

    ResourcePoolIterator() noexcept = default;

    explicit ResourcePoolIterator(Resource* start, Resource* end, Resource* resources) :
        _start(start),
        _end(end),
        _resources(resources) {
    }

    reference operator*() noexcept {
        return &_resources->obj;
    }

    const_reference operator*() const noexcept {
        return &_resources->obj;
    }

    pointer operator->() noexcept {
        return &_resources->obj;
    }

    const_pointer operator->() const noexcept {
        return &_resources->obj;
    }

    reference operator[](difference_type n) noexcept {
        return *(*this + n);
    }

    const_reference operator[](difference_type n) const noexcept {
        return *(*this + n);
    }

    ResourcePoolIterator& operator++() noexcept {
        moveOffset(1);
        return *this;
    }

    ResourcePoolIterator operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    ResourcePoolIterator& operator--() noexcept {
        moveOffset(-1);
        return *this;
    }

    ResourcePoolIterator operator--(int) noexcept {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    ResourcePoolIterator& operator+=(difference_type n) noexcept {
        moveOffset(n);
        return *this;
    }

    ResourcePoolIterator& operator-=(difference_type n) noexcept {
        moveOffset(-n);
        return *this;
    }

    ResourcePoolIterator operator+(difference_type n) const noexcept {
        auto tmp = *this;

        tmp += n;
        return tmp;
    }

    friend ResourcePoolIterator operator+(difference_type n, const ResourcePoolIterator& it) noexcept {
        return it + n;
    }

    ResourcePoolIterator operator-(difference_type n) const noexcept {
        auto tmp = *this;

        tmp -= n;
        return tmp;
    }

    difference_type operator-(const ResourcePoolIterator& other) const noexcept {
        if (!_resources) {
            return 0;
        }
        auto it               = other;
        auto end              = *this;
        difference_type count = 0;
        while (it++ != end) {
            ++count;
        }
        return count;
    }

    bool operator==(const ResourcePoolIterator& other) const noexcept {
        return _resources == other._resources;
    }

    bool operator!=(const ResourcePoolIterator& other) const noexcept {
        return !(*this == other);
    }

    bool operator<(const ResourcePoolIterator& other) const noexcept {
        return _resources < other._resources;
    }

    bool operator<=(const ResourcePoolIterator& other) const noexcept {
        return !(other < *this);
    }

    bool operator>(const ResourcePoolIterator& other) const noexcept {
        return other < *this;
    }

    bool operator>=(const ResourcePoolIterator& other) const noexcept {
        return !(*this < other);
    }

private:
    void moveOffset(difference_type n) noexcept {
        if (n > 0) {
            for (difference_type i = 0; i < n; ++i) {
                ++_resources;
                while (_resources->references == 0 && _resources < _end) {
                    ++_resources;
                }
            }
        } else if (n < 0) {
            n = -n;
            for (difference_type i = 0; i < n; ++i) {
                --_resources;
                while (_resources->references == 0 && _resources > _start) {
                    --_resources;
                }
            }
        }
    }

    Resource* _start{};
    Resource* _end{};
    Resource* _resources{};
};

template <typename T, typename H, bool Resizable = true>
class ResourcePool final {
private:
    struct Resource {
        gerium_uint16_t handle;
        gerium_uint16_t references;
        T obj;
    };

public:
    using base_handle_type = gerium_uint16_t;

    using value_type      = T;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

    using iterator       = ResourcePoolIterator<value_type, Resource>;
    using const_iterator = ResourcePoolIterator<value_type, Resource>;

    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    ResourcePool(size_type poolSize = 128) : _poolSize(0), _head(0), _handles(nullptr), _data(nullptr) {
        reserve(poolSize);
    }

    ~ResourcePool() {
        clear();
    }

    ResourcePool(const ResourcePool&) = delete;

    ResourcePool& operator=(const ResourcePool&) = delete;

    ResourcePool(ResourcePool&& other) noexcept :
        _poolSize(other._poolSize),
        _head(other._head),
        _handles(other._handles),
        _data(other._data) {
        other._poolSize = 0;
        other._head     = 0;
        other._handles  = nullptr;
        other._data     = nullptr;
    }

    ResourcePool& operator=(ResourcePool&& other) noexcept {
        if (this != &other) {
            clear();
            _poolSize       = other._poolSize;
            _head           = other._head;
            _handles        = other._handles;
            _data           = other._data;
            other._poolSize = 0;
            other._head     = 0;
            other._handles  = nullptr;
            other._data     = nullptr;
        }
        return *this;
    }

    bool empty() const noexcept {
        return size() == 0;
    }

    size_type size() const noexcept {
        return _head;
    }

    size_type capacity() const noexcept {
        return _poolSize;
    }

    void reserve(size_type newPoolSize) {
        assert(newPoolSize > _poolSize);

        auto handles =
            reinterpret_cast<base_handle_type*>(mi_reallocn(_handles, newPoolSize, sizeof(base_handle_type)));
        if (!handles) {
            throw std::bad_alloc();
        }

        Resource* data =
            reinterpret_cast<Resource*>(mi_recalloc_aligned(_data, newPoolSize, sizeof(Resource), alignof(Resource)));

        if (!data) {
            mi_free(handles);
            throw std::bad_alloc();
        }

        for (base_handle_type i = _poolSize; i < newPoolSize; ++i) {
            handles[i] = i;
        }

        _poolSize = newPoolSize;
        _handles  = handles;
        _data     = data;
    }

    void clear() noexcept {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (base_handle_type i = 0; i < _head; ++i) {
                access(H{ i }).~T();
            }
        }

        _head     = 0;
        _poolSize = 0;

        if (_data) {
            mi_free_aligned(_data, alignof(Resource));
            _data = nullptr;
        }

        if (_handles) {
            mi_free(_handles);
            _handles = nullptr;
        }
    }

    H obtain() {
        checkInit();
        assert(_head < std::numeric_limits<gerium_uint16_t>::max());

        if (_head >= _poolSize) {
            if constexpr (!Resizable) {
                throw std::bad_alloc();
            }
            reserve(_poolSize << 1);
        }

        const auto index = _handles[_head++];

        if constexpr (!std::is_trivially_default_constructible_v<T>) {
            new (access(H{ index })) T();
        }

        _data[index].handle     = index;
        _data[index].references = 1;

        return { index };
    }

    bool release(H handle) noexcept {
        checkInit();
        checkHandle(handle);

        if (--_data[handle.index].references == 0) {
            if constexpr (!std::is_trivially_destructible_v<T>) {
                access(handle)->~T();
            }

            std::memset(&_data[handle.index], 0, sizeof(Resource));

            _handles[--_head] = handle.index;
            return true;
        } else {
            return false;
        }
    }

    void releaseAll() noexcept {
        checkInit();

        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (gerium_uint16_t i = 0; i < _head; ++i) {
                access(H{ i }).~T();
            }
        }

        std::memset(_data, 0, _head * sizeof(Resource));

        _head = 0;

        for (gerium_uint16_t i = 0; i < _poolSize; ++i) {
            _handles[i] = i;
        }
    }

    T* addReference(H handle) noexcept {
        checkInit();
        checkHandle(handle);
        ++_data[handle.index].references;
        return &_data[handle.index].obj;
    }

    T* addReference(T* resource) noexcept {
        checkInit();
        constexpr auto offset = offsetof(Resource, obj);
        auto& owner           = *reinterpret_cast<Resource*>((char*) resource - offset);
        ++owner.references;
        return &owner.obj;
    }

    gerium_uint16_t references(H handle) const noexcept {
        checkInit();
        checkHandle(handle);
        return _data[handle.index].references;
    }

    gerium_uint16_t references(const T* resource) const noexcept {
        checkInit();
        constexpr auto offset = offsetof(Resource, obj);
        const auto& owner     = *reinterpret_cast<const Resource*>((const char*) resource - offset);
        return owner.references;
    }

    T* access(H handle) noexcept {
        checkInit();
        checkHandle(handle);
        return &_data[handle.index].obj;
    }

    const T* access(H handle) const noexcept {
        checkInit();
        checkHandle(handle);
        return &_data[handle.index].obj;
    }

    std::pair<H, T*> obtain_and_access() {
        const auto handle = obtain();
        return { handle, access(handle) };
    }

    Handle handle(const T* resource) const noexcept {
        constexpr auto offset = offsetof(Resource, obj);
        const auto& owner     = *reinterpret_cast<const Resource*>((const char*) resource - offset);
        return { owner.handle };
    }

    iterator begin() noexcept {
        auto start = findFirst();
        auto end   = findEnd(start);
        return iterator(start, end, start);
    }

    iterator end() noexcept {
        auto start = findFirst();
        auto end   = findEnd(start);
        return iterator(start, end, end);
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_iterator cbegin() const noexcept {
        return begin();
    }

    const_iterator cend() const noexcept {
        return end();
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(cbegin());
    }

    reference operator[](difference_type n) noexcept {
        return at(n);
    }

    const_reference operator[](difference_type n) const noexcept {
        return at(n);
    }

    reference at(difference_type n) noexcept {
        return *(begin() + n);
    }

    const_reference at(difference_type n) const noexcept {
        return *(begin() + n);
    }

    reference front() noexcept {
        return *begin();
    }

    const_reference front() const noexcept {
        return *begin();
    }

    reference back() noexcept {
        return *rbegin();
    }

    const_reference back() const noexcept {
        return *rbegin();
    }

private:
    void checkInit() const {
        assert(_data && "memory not allocated");
    }

    void checkHandle(H handle) const {
        assert(_head > 0 && "invalid handle");
        assert(handle.index < _poolSize && "invalid handle");
    }

    Resource* findFirst() noexcept {
        if (empty()) {
            return nullptr;
        }
        auto item = _data;
        while (item->references == 0) {
            ++item;
        }
        return item;
    }

    Resource* findEnd(Resource* first) noexcept {
        if (empty()) {
            return nullptr;
        }
        if (size() == 1) {
            return first + 1;
        }
        auto item = first;
        for (base_handle_type i = 1; i < _head; ++i) {
            ++item;
            while (item->references == 0) {
                ++item;
            }
        }
        return item + 1;
    }

    base_handle_type _poolSize;
    base_handle_type _head;
    base_handle_type* _handles;
    Resource* _data;
};

} // namespace gerium

#endif
