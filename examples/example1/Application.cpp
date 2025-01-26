#include "Application.hpp"
#include "components/Position.hpp"
#include "events/WindowStateEvent.hpp"
#include "services/GameService.hpp"
#include "services/InputService.hpp"
#include "services/RenderService.hpp"
#include "services/TimeService.hpp"

Application::Application() {
    check(gerium_logger_create("example1", &_logger));
}

Application::~Application() {
    if (_application) {
        gerium_application_destroy(_application);
        _application = nullptr;
    }
    if (_logger) {
        gerium_logger_destroy(_logger);
        _logger = nullptr;
    }
}

void Application::run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) noexcept {
    if constexpr (debugBuild) {
        gerium_logger_set_level_by_tag("gerium", GERIUM_LOGGER_LEVEL_ERROR);
    }

    try {
        check(gerium_application_create(title, width, height, &_application));
        gerium_application_set_background_wait(_application, true);

        gerium_application_set_frame_func(_application, frame, (gerium_data_t) this);
        gerium_application_set_state_func(_application, state, (gerium_data_t) this);

        auto result = gerium_application_run(_application);
        if (_error) {
            std::rethrow_exception(_error);
        }
        check(result);
    } catch (const std::exception& exc) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
        gerium_application_show_message(_application, "example1", exc.what());
    } catch (...) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
        gerium_application_show_message(_application, "example1", "unknown error");
    }
}

void Application::initialize() {
    _entityManager.registerComponent<Position>();
    _serviceManager.create(this);
    _serviceManager.addService<TimeService>();
    _serviceManager.addService<InputService>();
    _serviceManager.addService<GameService>();
    _serviceManager.addService<RenderService>();
}

void Application::uninitialize() {
    _serviceManager.destroy();
}

void Application::frame(gerium_uint64_t elapsedMs) {
    _eventManager.dispatch();
    _serviceManager.update(elapsedMs);
}

void Application::state(gerium_application_state_t state) {
    const auto stateStr = magic_enum::enum_name(state);
    gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_DEBUG, stateStr.data());

    gerium_uint16_t width, height;
    gerium_application_get_size(_application, &width, &height);

    switch (state) {
        case GERIUM_APPLICATION_STATE_INITIALIZE:
            initialize();
            break;
        case GERIUM_APPLICATION_STATE_UNINITIALIZE:
            uninitialize();
            break;
        case GERIUM_APPLICATION_STATE_GOT_FOCUS:
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
        case GERIUM_APPLICATION_STATE_VISIBLE:
        case GERIUM_APPLICATION_STATE_INVISIBLE:
        case GERIUM_APPLICATION_STATE_NORMAL:
        case GERIUM_APPLICATION_STATE_MINIMIZE:
        case GERIUM_APPLICATION_STATE_MAXIMIZE:
        case GERIUM_APPLICATION_STATE_FULLSCREEN:
        case GERIUM_APPLICATION_STATE_RESIZED:
            _eventManager.send<WindowStateEvent>(state, width, height);
            break;
        default:
            break;
    }
}

gerium_bool_t Application::frame(gerium_application_t application, gerium_data_t data, gerium_uint64_t elapsedMs) {
    auto app = (Application*) data;
    try {
        app->frame(elapsedMs);
    } catch (...) {
        app->uninitialize();
        app->_error = std::current_exception();
        return false;
    }
    return true;
}

gerium_bool_t Application::state(gerium_application_t application,
                                 gerium_data_t data,
                                 gerium_application_state_t state) {
    auto app = (Application*) data;
    try {
        app->state(state);
    } catch (...) {
        app->uninitialize();
        app->_error = std::current_exception();
        return false;
    }
    return true;
}
