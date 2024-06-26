#ifndef GERIUM_OBJECT_HPP
#define GERIUM_OBJECT_HPP

#include "Exceptions.hpp"

namespace gerium {

class Object {
public:
    Object() noexcept;
    virtual ~Object() = default;

    void reference() noexcept;
    void destroy() noexcept;

template <typename D>
    gerium_state_t invoke(std::function<void(D*)>&& func) noexcept;

    template <typename D, typename T, typename... Args>
    static gerium_state_t create(T*& obj, Args&&... args) noexcept;

private:
    gerium_sint32_t _refCount;
};

template <typename D>
inline gerium_state_t Object::invoke(std::function<void(D*)>&& func) noexcept {
    static_assert(std::is_base_of_v<Object, D>, "D must inheritance from T");
    try {
        func(alias_cast<D*>(this));
        return GERIUM_STATE_SUCCESS;
    } catch (const Exception& exc) {
        return exc.state();
    } catch (...) {
        return GERIUM_STATE_UNKNOWN_ERROR;
    }
}

template <typename D, typename T, typename... Args>
gerium_inline gerium_state_t Object::create(T*& obj, Args&&... args) noexcept {
    static_assert(std::is_base_of_v<T, D>, "D must inheritance from T");
    try {
        obj = new (std::nothrow) D(args...);
        return obj ? GERIUM_STATE_SUCCESS : GERIUM_STATE_OUT_OF_MEMORY;
    } catch (const Exception& exc) {
        return exc.state();
    } catch (...) {
        return GERIUM_STATE_UNKNOWN_ERROR;
    }
}

} // namespace gerium

#endif
