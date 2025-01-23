#ifndef WINDOW_STATE_EVENT_HPP
#define WINDOW_STATE_EVENT_HPP

#include "Event.hpp"

class WindowStateEvent : public Event {
public:
    WindowStateEvent(gerium_application_state_t state, gerium_uint16_t width, gerium_uint16_t height) noexcept;

    gerium_application_state_t state() const noexcept {
        return _state;
    }

    gerium_uint16_t width() const noexcept {
        return _width;
    }

    gerium_uint16_t height() const noexcept {
        return _height;
    }

private:
    gerium_application_state_t _state{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
};

#endif
