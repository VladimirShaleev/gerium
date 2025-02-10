#include "TimeService.hpp"

void TimeService::setMultiply(gerium_float64_t multiply) noexcept {
    _multiply = multiply;
}

gerium_float64_t TimeService::multiply() const noexcept {
    return _multiply;
}

gerium_uint64_t TimeService::elapsedMs() const noexcept {
    return _elapsedMs;
}

gerium_uint64_t TimeService::absoluteMs() const noexcept {
    return _absoluteMs;
}

gerium_float64_t TimeService::elapsed() const noexcept {
    return _elapsed;
}

gerium_float64_t TimeService::absolute() const noexcept {
    return _absolute;
}

void TimeService::start() {
}

void TimeService::stop() {
}

void TimeService::update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) {
    // TODO:
    // auto absolute = manager().absoluteMs() * _multiply;

    // _elapsedMs  = gerium_uint64_t(absolute) - _absoluteMs;
    // _absoluteMs = gerium_uint64_t(absolute);
    // _elapsed    = _elapsedMs * 0.001;
    // _absolute   = _absoluteMs * 0.001;
}
