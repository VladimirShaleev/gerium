#ifndef GERIUM_EXCEPTIONS_HPP
#define GERIUM_EXCEPTIONS_HPP

#include "Gerium.hpp"

#define GERIUM_BEGIN_SAFE_BLOCK try {
#define GERIUM_END_SAFE_BLOCK                     \
    }                                             \
    catch (const Exception& exc) {                \
        return exc.result();                      \
    }                                             \
    catch (const std::bad_alloc&) {               \
        return GERIUM_RESULT_ERROR_OUT_OF_MEMORY; \
    }                                             \
    catch (...) {                                 \
        return GERIUM_RESULT_ERROR_UNKNOWN;       \
    }                                             \
    return GERIUM_RESULT_SUCCESS;
#define GERIUM_END_SAFE_VOID_BLOCK \
    }                              \
    catch (...) {                  \
    }

#ifdef NDEBUG
# define GERIUM_ASSERT_ARG(expression) ((void) 0)
#else
# define GERIUM_ASSERT_ARG(expression)                \
     if (!!(expression)) {                            \
         return GERIUM_RESULT_ERROR_INVALID_ARGUMENT; \
     }
#endif

namespace gerium {

class Exception : public std::runtime_error {
public:
    Exception(gerium_result_t result) : std::runtime_error(gerium_result_to_string(result)), _result(result) {
    }

    gerium_result_t result() const noexcept {
        return _result;
    }

private:
    const gerium_result_t _result;
};

} // namespace gerium

#endif
