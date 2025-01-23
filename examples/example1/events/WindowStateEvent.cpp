#include "WindowStateEvent.hpp"

WindowStateEvent::WindowStateEvent(gerium_application_state_t state,
                                   gerium_uint16_t width,
                                   gerium_uint16_t height) noexcept :
    _state(state),
    _width(width),
    _height(height) {
}
