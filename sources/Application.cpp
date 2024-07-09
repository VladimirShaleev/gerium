#include "Application.hpp"

namespace gerium {

Application::Application() noexcept :
    _frameFunc(nullptr),
    _stateFunc(nullptr),
    _frameData(nullptr),
    _stateData(nullptr),
    _backgroundWait(false),
    _prevState(GERIUM_APPLICATION_STATE_UNKNOWN) {
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

gerium_result_t Application::getDisplayInfo(gerium_uint32_t& displayCount,
                                            gerium_display_info_t* displays) const noexcept {
    return invoke<Application>([&displayCount, &displays](auto obj) {
        obj->onGetDisplayInfo(displayCount, displays);
    });
}

bool Application::isFullscreen() const noexcept {
    return onIsFullscreen();
}

gerium_result_t Application::fullscreen(bool fullscreen,
                                        gerium_uint32_t displayId,
                                        const gerium_display_mode_t* mode) noexcept {
    return invoke<Application>([fullscreen, displayId, mode](auto obj) {
        if (fullscreen != obj->isFullscreen()) {
            obj->onFullscreen(fullscreen, displayId, mode);
        }
    });
}

gerium_application_style_flags_t Application::getStyle() const noexcept {
    return onGetStyle();
}

void Application::setStyle(gerium_application_style_flags_t style) noexcept {
    onSetStyle(style);
}

void Application::getMinSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    onGetMinSize(width, height);
}

void Application::getMaxSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    onGetMaxSize(width, height);
}

void Application::getSize(gerium_uint16_t* width, gerium_uint16_t* height) const noexcept {
    onGetSize(width, height);
}

void Application::setMinSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    onSetMinSize(width, height);
}

void Application::setMaxSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    onSetMaxSize(width, height);
}

void Application::setSize(gerium_uint16_t width, gerium_uint16_t height) noexcept {
    onSetSize(width, height);
}

gerium_utf8_t Application::getTitle() const noexcept {
    return onGetTitle();
}

void Application::setTitle(gerium_utf8_t title) noexcept {
    onSetTitle(title ? title : "");
}

bool Application::getBackgroundWait() const noexcept {
    return _backgroundWait;
}

void Application::setBackgroundWait(bool enable) noexcept {
    _backgroundWait = enable;
}

gerium_result_t Application::run() noexcept {
    return invoke<Application>([](auto obj) {
        obj->onRun();
    });
}

void Application::exit() noexcept {
    onExit();
}

void Application::changeState(gerium_application_state_t newState) {
    if (newState != _prevState || newState == GERIUM_APPLICATION_STATE_RESIZE) {
        _prevState = newState;
        if (!callStateFunc(newState)) {
            error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }
    }
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

gerium_result_t gerium_application_get_display_info(gerium_application_t application,
                                                    gerium_uint32_t* display_count,
                                                    gerium_display_info_t* displays) {
    assert(application);
    return alias_cast<Application*>(application)->getDisplayInfo(*display_count, displays);
}

gerium_bool_t gerium_application_is_fullscreen(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->isFullscreen() ? 1 : 0;
}

gerium_result_t gerium_application_fullscreen(gerium_application_t application,
                                              gerium_bool_t fullscreen,
                                              gerium_uint32_t display_id,
                                              const gerium_display_mode_t* mode) {
    assert(application);
    return alias_cast<Application*>(application)->fullscreen(fullscreen, display_id, mode);
}

gerium_application_style_flags_t gerium_application_get_style(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->getStyle();
}

void gerium_application_set_style(gerium_application_t application, gerium_application_style_flags_t style) {
    assert(application);
    alias_cast<Application*>(application)->setStyle(style);
}

void gerium_application_get_min_size(gerium_application_t application,
                                     gerium_uint16_t* width,
                                     gerium_uint16_t* height) {
    assert(application);
    alias_cast<Application*>(application)->getMinSize(width, height);
}

void gerium_application_set_min_size(gerium_application_t application, gerium_uint16_t width, gerium_uint16_t height) {
    assert(application);
    alias_cast<Application*>(application)->setMinSize(width, height);
}

void gerium_application_get_max_size(gerium_application_t application,
                                     gerium_uint16_t* width,
                                     gerium_uint16_t* height) {
    assert(application);
    alias_cast<Application*>(application)->getMaxSize(width, height);
}

void gerium_application_set_max_size(gerium_application_t application, gerium_uint16_t width, gerium_uint16_t height) {
    assert(application);
    alias_cast<Application*>(application)->setMaxSize(width, height);
}

void gerium_application_get_size(gerium_application_t application, gerium_uint16_t* width, gerium_uint16_t* height) {
    assert(application);
    alias_cast<Application*>(application)->getSize(width, height);
}

void gerium_application_set_size(gerium_application_t application, gerium_uint16_t width, gerium_uint16_t height) {
    assert(application);
    alias_cast<Application*>(application)->setSize(width, height);
}

gerium_utf8_t gerium_application_get_title(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->getTitle();
}

void gerium_application_set_title(gerium_application_t application, gerium_utf8_t title) {
    assert(application);
    alias_cast<Application*>(application)->setTitle(title);
}

gerium_bool_t gerium_application_get_background_wait(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->getBackgroundWait();
}

void gerium_application_set_background_wait(gerium_application_t application, gerium_bool_t enable) {
    assert(application);
    alias_cast<Application*>(application)->setBackgroundWait(enable);
}

gerium_result_t gerium_application_run(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->run();
}

void gerium_application_exit(gerium_application_t application) {
    assert(application);
    alias_cast<Application*>(application)->exit();
}
