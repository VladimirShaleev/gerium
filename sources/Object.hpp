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
    [[noreturn]] static void error(gerium_result_t result);

private:
    gerium_sint32_t _refCount;
};

template <typename D, typename T, typename... Args>
gerium_inline gerium_result_t Object::create(T*& obj, Args&&... args) noexcept {
    static_assert(std::is_base_of_v<T, D>, "D must inheritance from T");
    try {
        obj = new D(args...);
        return GERIUM_RESULT_SUCCESS;
    } catch (const Exception& exc) {
        return exc.result();
    } catch (const std::bad_alloc&) {
        return GERIUM_RESULT_ERROR_OUT_OF_MEMORY;
    } catch (...) {
        return GERIUM_RESULT_ERROR_UNKNOWN;
    }
}

} // namespace gerium

#endif
