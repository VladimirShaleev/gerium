#ifndef GERIUM_OBJECT_PTR_HPP
#define GERIUM_OBJECT_PTR_HPP

#include "Object.hpp"

namespace gerium {

template <typename T>
class ObjectPtr final {
public:
    static_assert(std::is_base_of_v<Object, T>);

    constexpr ObjectPtr() noexcept : _obj(nullptr) {
    }

    constexpr ObjectPtr(std::nullptr_t) noexcept : _obj(nullptr) {
    }

    ObjectPtr(T* obj, bool addReference = true) noexcept : _obj(obj) {
        if (addReference) {
            reference();
        }
    }

    ~ObjectPtr() {
        destroy();
    }

    ObjectPtr(const ObjectPtr& obj) noexcept : _obj(obj._obj) {
        reference();
    }

    ObjectPtr(ObjectPtr&& obj) noexcept : _obj(obj._obj) {
        obj._obj = nullptr;
    }

    ObjectPtr& operator=(nullptr_t) noexcept {
        destroy();
        return *this;
    }

    ObjectPtr& operator=(T* obj) noexcept {
        reset(obj);
        return *this;
    }

    ObjectPtr& operator=(const ObjectPtr& obj) noexcept {
        reset(obj._obj);
        return *this;
    }

    ObjectPtr& operator=(ObjectPtr&& obj) noexcept {
        if (_obj != obj._obj) {
            destroy();
            _obj = obj._obj;
            obj._obj = nullptr;
        }
        return *this;
    }

    T& operator*() noexcept {
        return *_obj;
    }

    const T& operator*() const noexcept {
        return *_obj;
    }

    T* operator->() noexcept {
        return _obj;
    }

    const T* operator->() const noexcept {
        return _obj;
    }

    explicit operator bool() const noexcept {
        return _obj != nullptr;
    }

    void reset() noexcept {
        destroy();
    }

    void reset(T* obj) noexcept {
        if (_obj != obj) {
            destroy();
            _obj = obj;
            reference();
        }
    }

    void swap(ObjectPtr& obj) noexcept {
        std::swap(_obj, obj._obj);
    }

    T* get() noexcept {
        return _obj;
    }

    const T* get() const noexcept {
        return _obj;
    }

private:
    void reference() noexcept {
        if (_obj) {
            _obj->reference();
        }
    }

    void destroy() noexcept {
        if (_obj) {
            _obj->destroy();
        }
    }

    T* _obj;
};

template <typename D, typename B, typename... Args>
[[nodiscard]] gerium_inline ObjectPtr<B> createObjectPtr(Args&&... args) {
    static_assert(std::is_base_of_v<B, D>, "D must inheritance from B");
    auto obj = new D(args...);
    return ObjectPtr(alias_cast<B*>(obj), false);
}

} // namespace gerium

#endif
