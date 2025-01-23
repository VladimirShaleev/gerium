#include "InputService.hpp"
#include "../Application.hpp"

bool InputService::isPressScancode(gerium_scancode_t scancode) const noexcept {
    return gerium_application_is_press_scancode(application().handle(), scancode);
}

void InputService::start() {
}

void InputService::stop() {
    _events.clear();
}

void InputService::update() {
    gerium_event_t event;
    while (gerium_application_poll_events(application().handle(), &event)) {
        _events.push_back(event);
    }
}
