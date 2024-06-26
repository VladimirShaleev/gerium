#ifndef GERIUM_EXCEPTIONS_HPP
#define GERIUM_EXCEPTIONS_HPP

#include "Gerium.hpp"

namespace gerium {

class Exception : public std::runtime_error {
public:
    Exception(gerium_state_t state, const char* message) : std::runtime_error(message), _state(state) {
    }

    Exception(gerium_state_t state, const std::string& message) : std::runtime_error(message), _state(state) {
    }

    gerium_state_t state() const noexcept {
        return _state;
    }

private:
    const gerium_state_t _state;
};

} // namespace gerium

#endif
