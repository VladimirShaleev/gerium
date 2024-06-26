#ifndef GERIUM_EXCEPTIONS_HPP
#define GERIUM_EXCEPTIONS_HPP

#include "Gerium.hpp"

namespace gerium {

class Exception : public std::runtime_error {
public:
    Exception(gerium_result_t result, const char* message) : std::runtime_error(message), _result(result) {
    }

    Exception(gerium_result_t result, const std::string& message) : std::runtime_error(message), _result(result) {
    }

    gerium_result_t result() const noexcept {
        return _result;
    }

private:
    const gerium_result_t _result;
};

} // namespace gerium

#endif
