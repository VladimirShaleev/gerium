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

    template <typename D, typename T, typename... Args>
    static gerium_result_t create(T*& obj, Args&&... args) noexcept;

protected:
    template <typename D>
    gerium_result_t invoke(std::function<void(D*)>&& func) const noexcept;

private:
    gerium_sint32_t _refCount;
};

template <typename D, typename T, typename... Args>
gerium_inline gerium_result_t Object::create(T*& obj, Args&&... args) noexcept {
    static_assert(std::is_base_of_v<T, D>, "D must inheritance from T");
    try {
        obj = new (std::nothrow) D(args...);
        return obj ? GERIUM_RESULT_SUCCESS : GERIUM_RESULT_ERROR_OUT_OF_MEMORY;
    } catch (const Exception& exc) {
        return exc.result();
    } catch (...) {
        return GERIUM_RESULT_ERROR_UNKNOWN;
    }
}

template <typename D>
inline gerium_result_t Object::invoke(std::function<void(D*)>&& func) const noexcept {
    static_assert(std::is_base_of_v<Object, D>, "D must inheritance from T");
    try {
        func(alias_cast<D*>(this));
        return GERIUM_RESULT_SUCCESS;
    } catch (const Exception& exc) {
        return exc.result();
    } catch (...) {
        return GERIUM_RESULT_ERROR_UNKNOWN;
    }
}

} // namespace gerium

#endif
