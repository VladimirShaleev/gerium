#include "Application.hpp"

namespace gerium {

Application::Application() noexcept :
    _logger(Logger::create("gerium:application")),
    _frameFunc(nullptr),
    _stateFunc(nullptr),
    _frameData(nullptr),
    _stateData(nullptr),
    _backgroundWait(false),
    _isShowCursor(true),
    _workerThreadCount(0),
    _currentState(GERIUM_APPLICATION_STATE_UNKNOWN),
    _callbackStateFailed(false),
    _keys(),
    _eventPos(0),
    _eventCount(0) {
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

void Application::getDisplayInfo(gerium_uint32_t& displayCount, gerium_display_info_t* displays) const {
    onGetDisplayInfo(displayCount, displays);
}

bool Application::isFullscreen() const noexcept {
    return onIsFullscreen();
}

void Application::fullscreen(bool fullscreen, gerium_uint32_t displayId, const gerium_display_mode_t* mode) {
    if (fullscreen != isFullscreen()) {
        onFullscreen(fullscreen, displayId, mode);
    }
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

bool Application::isShowCursor() const noexcept {
    return _isShowCursor;
}

void Application::showCursor(bool show) noexcept {
    if (!!_isShowCursor != show) {
        _isShowCursor = show;
        onShowCursor(show);
    }
}

void Application::run() {
    _workerThreadCount = (gerium_uint32_t) marl::Thread::numLogicalCPUs();

    marl::Scheduler::Config cfg;
    cfg.setWorkerThreadCount((int) _workerThreadCount);

    marl::Scheduler scheduler(cfg);
    scheduler.bind();
    defer(scheduler.unbind());

    onRun();
}

void Application::exit() noexcept {
    onExit();
}

bool Application::pollEvents(gerium_event_t& event) noexcept {
    if (_eventPos == _eventCount) {
        return false;
    }

    marl::lock lock(_eventSync);

    if (_eventPos == _eventCount) {
        return false;
    }

    const auto currentIndex = _eventPos % std::size(_events);

    event = _events[currentIndex];
    ++_eventPos;

    return true;
}

bool Application::isPressScancode(gerium_scancode_t scancode) const noexcept {
    if (const auto index = (int) scancode; index < std::size(_keys)) {
        return _keys[index];
    }
    return false;
}

void Application::execute(gerium_application_executor_func_t callback, gerium_data_t data) noexcept {
    marl::schedule([this, callback, data]() {
        callback(this, data);
    });
}

void Application::showMessage(gerium_utf8_t title, gerium_utf8_t message) noexcept {
    onShowMessage(title ? title : "Gerium", message ? message : "Unknown message");
}

bool Application::isRunning() const noexcept {
    return onIsRunning();
}

gerium_uint32_t Application::workerThreadCount() const noexcept {
    return _workerThreadCount;
}

void Application::initImGui() {
    onInitImGui();
}

void Application::shutdownImGui() {
    onShutdownImGui();
}

void Application::newFrameImGui() {
    onNewFrameImGui();
}

gerium_application_state_t Application::currentState() const noexcept {
    return _currentState;
}

gerium_bool_t Application::callbackStateFailed() const noexcept {
    return _callbackStateFailed;
}

void Application::changeState(gerium_application_state_t newState, bool noThrow) {
    if (newState != _currentState || newState == GERIUM_APPLICATION_STATE_RESIZE) {
        _currentState = newState;
        if (!callStateFunc(newState)) {
            if (noThrow) {
                _callbackStateFailed = true;
            } else {
                error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
            }
        }
    }
}

bool Application::callFrameFunc(gerium_uint64_t elapsedMs) noexcept {
    return _frameFunc ? _frameFunc(this, _frameData, elapsedMs) : true;
}

bool Application::callStateFunc(gerium_application_state_t state) noexcept {
    return _stateFunc ? _stateFunc(this, _stateData, state) : true;
}

void Application::clearStates(gerium_uint64_t timestamp) noexcept {
    for (size_t i = 0; i < std::size(_keys); ++i) {
        if (_keys[i]) {
            _keys[i] = 0;

            gerium_event_t event{};
            event.type              = GERIUM_EVENT_TYPE_KEYBOARD;
            event.timestamp         = timestamp;
            event.keyboard.scancode = (gerium_scancode_t) i;
            event.keyboard.state    = GERIUM_KEY_STATE_RELEASED;
            addEvent(event);
        }
    }
}

void Application::setKeyState(gerium_scancode_t scancode, bool press) noexcept {
    if (const auto index = (int) scancode; index < std::size(_keys)) {
        _keys[index] = press;
    }
}

void Application::addEvent(const gerium_event_t& event) noexcept {
    marl::lock lock(_eventSync);

    const auto nextIndex    = _eventCount % std::size(_events);
    const auto currentIndex = _eventPos % std::size(_events);
    const auto diff         = _eventCount - _eventPos;

    if (diff >= std::size(_events)) {
        _logger->print(GERIUM_LOGGER_LEVEL_WARNING, "The input event queue is overflow. Old events will be removed");
        ++_eventPos;
    }

    _events[nextIndex] = event;
    ++_eventCount;
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
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Application*>(application)->getDisplayInfo(*display_count, displays);
    GERIUM_END_SAFE_BLOCK
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
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Application*>(application)->fullscreen(fullscreen, display_id, mode);
    GERIUM_END_SAFE_BLOCK
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

gerium_bool_t gerium_application_is_show_cursor(gerium_application_t application) {
    assert(application);
    return alias_cast<Application*>(application)->isShowCursor();
}

void gerium_application_show_cursor(gerium_application_t application, gerium_bool_t show) {
    assert(application);
    alias_cast<Application*>(application)->showCursor(show);
}

gerium_result_t gerium_application_run(gerium_application_t application) {
    assert(application);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<Application*>(application)->run();
    GERIUM_END_SAFE_BLOCK
}

void gerium_application_exit(gerium_application_t application) {
    assert(application);
    alias_cast<Application*>(application)->exit();
}

gerium_bool_t gerium_application_poll_events(gerium_application_t application, gerium_event_t* event) {
    assert(application);
    assert(event);
    return alias_cast<Application*>(application)->pollEvents(*event);
}

gerium_bool_t gerium_application_is_press_scancode(gerium_application_t application, gerium_scancode_t scancode) {
    assert(application);
    return alias_cast<Application*>(application)->isPressScancode(scancode);
}

void gerium_application_execute(gerium_application_t application,
                                gerium_application_executor_func_t callback,
                                gerium_data_t data) {
    assert(application);
    assert(callback);
    return alias_cast<Application*>(application)->execute(callback, data);
}

void gerium_application_show_message(gerium_application_t application, gerium_utf8_t title, gerium_utf8_t message) {
    assert(application);
    return alias_cast<Application*>(application)->showMessage(title, message);
}
