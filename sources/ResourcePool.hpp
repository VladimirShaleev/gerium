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

template <int Size>
struct BaseTypeFromSize;

template <>
struct BaseTypeFromSize<2> {
    using type = gerium_uint16_t;
};

template <>
struct BaseTypeFromSize<4> {
    using type = gerium_uint32_t;
};

template <typename H>
struct BaseType {
    using type = BaseTypeFromSize<sizeof(H)>::type;
};

constexpr Handle Undefined = Handle{ static_cast<Handle>(-1) };

template <typename T, typename BaseH, typename Resource>
class ResourcePoolIterator {
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = T;
    using difference_type   = ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;
    using const_pointer     = const value_type*;
    using const_reference   = const value_type&;

    ResourcePoolIterator() noexcept = default;

    explicit ResourcePoolIterator(Resource* resources, BaseH* handles, BaseH offset) :
        _resources(resources),
        _handles(handles),
        _offset(offset) {
    }

    reference operator*() noexcept {
        return _resources[_handles[_offset]];
    }

    const_reference operator*() const noexcept {
        return _resources[_handles[_offset]];
    }

    pointer operator->() noexcept {
        return &_resources[_handles[_offset]];
    }

    const_pointer operator->() const noexcept {
        return &_resources[_handles[_offset]];
    }

    reference operator[](difference_type n) noexcept {
        return *(*this + n);
    }

    const_reference operator[](difference_type n) const noexcept {
        return *(*this + n);
    }

    ResourcePoolIterator& operator++() noexcept {
        ++_offset;
        return *this;
    }

    ResourcePoolIterator operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    ResourcePoolIterator& operator--() noexcept {
        --_offset;
        return *this;
    }

    ResourcePoolIterator operator--(int) noexcept {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    ResourcePoolIterator& operator+=(difference_type n) noexcept {
        _offset += (BaseH) n;
        return *this;
    }

    ResourcePoolIterator& operator-=(difference_type n) noexcept {
        _offset -= (BaseH) n;
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
        return difference_type(_offset - other._offset);
    }

    bool operator==(const ResourcePoolIterator& other) const noexcept {
        return _offset == other._offset;
    }

    bool operator!=(const ResourcePoolIterator& other) const noexcept {
        return !(*this == other);
    }

    bool operator<(const ResourcePoolIterator& other) const noexcept {
        return _offset < other._offset;
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
    Resource* _resources{};
    BaseH* _handles{};
    BaseH _offset{};
};

template <typename T, typename H, bool Resizable = true>
class ResourcePool final {
public:
    using base_type = BaseType<H>::type;

    using value_type      = T;
    using size_type       = size_t;
    using difference_type = ptrdiff_t;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

    using iterator       = ResourcePoolIterator<value_type, base_type, T>;
    using const_iterator = ResourcePoolIterator<value_type, base_type, T>;

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

        auto handles = reinterpret_cast<base_type*>(mi_reallocn(_handles, newPoolSize, sizeof(base_type)));
        if (!handles) {
            throw std::bad_alloc();
        }

        T* data =
            reinterpret_cast<T*>(std::is_trivial_v<T> ? mi_recalloc_aligned(_data, newPoolSize, sizeof(T), alignof(T))
                                                      : mi_realloc_aligned(_data, newPoolSize * sizeof(T), alignof(T)));
        if (!data) {
            mi_free(handles);
            throw std::bad_alloc();
        }

        for (base_type i = _poolSize; i < newPoolSize; ++i) {
            handles[i] = i;
        }

        _poolSize = newPoolSize;
        _handles  = handles;
        _data     = data;
    }

    void clear() noexcept {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (base_type i = 0; i < _head; ++i) {
                access(H{ i }).~T();
            }
        }

        _head     = 0;
        _poolSize = 0;

        if (_data) {
            mi_free_aligned(_data, alignof(T));
            _data = nullptr;
        }

        if (_handles) {
            mi_free(_handles);
            _handles = nullptr;
        }
    }

    H obtain() {
        checkInit();
        assert(_head < std::numeric_limits<base_type>::max());

        if (_head >= _poolSize) {
            if constexpr (!Resizable) {
                throw std::bad_alloc();
            }
            reserve(_poolSize << 1);
        }

        const auto index = _handles[_head++];

        if constexpr (!std::is_trivially_default_constructible_v<T>) {
            new (&access(H{ index })) T();
        }

        return { index };
    }

    void release(H handle) noexcept {
        checkInit();
        checkHandle(handle);

        if constexpr (!std::is_trivially_destructible_v<T>) {
            access(handle).~T();
        }

        if constexpr (std::is_trivial_v<T>) {
            std::memset(&_data[handle.index], 0, sizeof(T));
        }

        _handles[handle.index] = _handles[--_head];
        _handles[_head]        = handle.index;
    }

    void releaseAll() noexcept {
        checkInit();

        if constexpr (!std::is_trivially_destructible_v<T>) {
            for (base_type i = 0; i < _head; ++i) {
                access(H{ i }).~T();
            }
        }

        if constexpr (std::is_trivial_v<T>) {
            std::memset(_data, 0, _head * sizeof(T));
        }

        _head = 0;

        for (base_type i = 0; i < _poolSize; ++i) {
            _handles[i] = i;
        }
    }

    T& access(H handle) noexcept {
        checkInit();
        checkHandle(handle);
        return _data[handle.index];
    }

    const T& access(H handle) const noexcept {
        checkInit();
        checkHandle(handle);
        return _data[handle.index];
    }

    std::pair<H, T&> obtain_and_access() {
        const auto handle = obtain();
        return { handle, access(handle) };
    }

    iterator begin() noexcept {
        return iterator(_data, _handles, 0);
    }

    iterator end() noexcept {
        return iterator(_data, _handles, _head);
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

    base_type _poolSize;
    base_type _head;
    base_type* _handles;
    T* _data;
};

} // namespace gerium

#endif
