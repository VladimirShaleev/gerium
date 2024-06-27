#include "Application.hpp"

namespace gerium {

Application::Application() noexcept :
    _frameFunc(nullptr),
    _stateFunc(nullptr),
    _frameData(nullptr),
    _stateData(nullptr) {
}

gerium_runtime_platform_t Application::getPlatform() const noexcept {
    return onGetPlatform();
}

gerium_application_frame_func_t Application::getFrameFunc(gerium_data_t* data) const noexcept {
    if (data) {
        *data = _frameData;
    }
    return _frameFunc;
}

gerium_application_state_func_t Application::getStateFunc(gerium_data_t* data) const noexcept {
    if (data) {
        *data = _stateData;
    }
    return _stateFunc;
}

void Application::setFrameFunc(gerium_application_frame_func_t callback, gerium_data_t data) noexcept {
    _frameFunc = callback;
    _frameData = data;
}

void Application::setStateFunc(gerium_application_state_func_t callback, gerium_data_t data) noexcept {
    _stateFunc = callback;
    _stateData = data;
}

bool Application::getFullscreen() const noexcept {
    return onGetFullscreen();
}

void Application::setFullscreen(bool fullscreen) noexcept {
    onSetFullscreen(fullscreen);
}

gerium_result_t Application::run() noexcept {
    return invoke<Application>([](auto obj) {
        obj->onRun();
    });
}

void Application::exit() noexcept {
    onExit();
}

bool Application::callFrameFunc(gerium_float32_t elapsed) noexcept {
    return _frameFunc ? _frameFunc(this, _frameData, elapsed) : true;
}

bool Application::callStateFunc(gerium_application_state_t state) noexcept {
    return _stateFunc ? _stateFunc(this, _stateData, state) : true;
}

} // namespace gerium

using namespace gerium;

gerium_application_t gerium_application_reference(gerium_application_t application) {
    assert(application);
    application->reference();
    return application;
}

void gerium_application_destroy(gerium_application_t application) {
    if (application) {
        application->destroy();
    }
}

gerium_runtime_platform_t gerium_application_get_platform(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->getPlatform();
}

gerium_application_frame_func_t gerium_application_get_frame_func(gerium_application_t application,
                                                                  gerium_data_t* data) {
    assert(application);
    return alias_cast<Application*>(application)->getFrameFunc(data);
}

void gerium_application_set_frame_func(gerium_application_t application,
                                       gerium_application_frame_func_t callback,
                                       gerium_data_t data) {
    assert(application);
    alias_cast<Application*>(application)->setFrameFunc(callback, data);
}

gerium_application_state_func_t gerium_application_get_state_func(gerium_application_t application,
                                                                  gerium_data_t* data) {
    assert(application);
    return alias_cast<Application*>(application)->getStateFunc(data);
}

void gerium_application_set_state_func(gerium_application_t application,
                                       gerium_application_state_func_t callback,
                                       gerium_data_t data) {
    assert(application);
    alias_cast<Application*>(application)->setStateFunc(callback, data);
}

gerium_bool_t gerium_application_get_fullscreen(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->getFullscreen() ? 1 : 0;
}

void gerium_application_set_fullscreen(gerium_application_t application, gerium_bool_t fullscreen) {
    assert(application);
    alias_cast<Application*>(application)->setFullscreen(fullscreen);
}

gerium_result_t gerium_application_run(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->run();
}

void gerium_application_exit(gerium_application_t application) {
    assert(application);
    alias_cast<Application*>(application)->exit();
}
