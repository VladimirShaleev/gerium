#include "Signal.hpp"

namespace gerium {

Signal::Signal() noexcept : _event(marl::Event::Mode::Manual) {
}

bool Signal::isSet() const noexcept {
    return _event.isSignalled();
}

void Signal::set() noexcept {
    _event.signal();
}

void Signal::wait() noexcept {
    _event.wait();
}

void Signal::clear() noexcept {
    _event.clear();
}

} // namespace gerium

using namespace gerium;

gerium_result_t gerium_signal_create(gerium_signal_t* signal) {
    assert(signal);
    return Object::create<Signal>(*signal);
}

gerium_signal_t gerium_signal_reference(gerium_signal_t signal) {
    assert(signal);
    signal->reference();
    return signal;
}

void gerium_signal_destroy(gerium_signal_t signal) {
    if (signal) {
        signal->destroy();
    }
}

gerium_bool_t gerium_signal_is_set(gerium_signal_t signal) {
    assert(signal);
    return alias_cast<Signal*>(signal)->isSet();
}

void gerium_signal_set(gerium_signal_t signal) {
    assert(signal);
    return alias_cast<Signal*>(signal)->set();
}

void gerium_signal_wait(gerium_signal_t signal) {
    assert(signal);
    return alias_cast<Signal*>(signal)->wait();
}

void gerium_signal_clear(gerium_signal_t signal) {
    assert(signal);
    return alias_cast<Signal*>(signal)->clear();
}
