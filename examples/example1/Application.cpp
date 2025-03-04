#include "Application.hpp"
#include "services/GameService.hpp"
#include "services/InputService.hpp"
#include "services/PhysicsService.hpp"
#include "services/RenderService.hpp"
#include "services/SceneService.hpp"
#include "services/TimeService.hpp"

#include "components/Camera.hpp"
#include "components/Settings.hpp"
#include "events/AddModelEvent.hpp"

#include "Model.hpp"
#include "Snapshot.hpp"

using namespace entt::literals;

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
    gerium_logger_set_level_by_tag("gerium", GERIUM_LOGGER_LEVEL_VERBOSE);
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

void Application::addModel(const hashed_string_owner& name,
                           const entt::hashed_string& model,
                           const glm::vec3& position,
                           const glm::quat& rotation,
                           const glm::vec3& scale) {
    dispatcher().enqueue<AddModelEvent>(entt::null, name, model, position, rotation, scale);
}

void Application::initialize() {
    _entityRegistry.ctx().emplace<Settings>(Settings{});

    _serviceManager.create(this);
    _serviceManager.addService<TimeService>();
    _serviceManager.addService<InputService>();
    _serviceManager.addService<GameService>();
    _serviceManager.addService<PhysicsService>();
    _serviceManager.addService<SceneService>();
    _serviceManager.addService<RenderService>();

    auto rotate = glm::rotate(glm::identity<glm::quat>(), glm::radians(150.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    addModel("main_truck"_hs, MODEL_TRUCK_ID, glm::vec3(2.0f, 0.0f, -8.0f), rotate);
    addModel("s0"_hs, MODEL_FLOOR_01_ID, glm::vec3(2.0f, -8.0f, -4.0f));
    addModel("s1"_hs, MODEL_FLOOR_01_ID, glm::vec3(2.0f, -8.0f, -8.0f));
    addModel("s2"_hs, MODEL_FLOOR_01_ID, glm::vec3(2.0f, -8.0f, -12.0f));
    addModel("s3"_hs, MODEL_FLOOR_01_ID, glm::vec3(2.0f, -10.0f, -16.0f));
    addModel("s4"_hs, MODEL_FLOOR_01_ID, glm::vec3(2.0f, -10.0f, -20.0f));
    addModel("s5"_hs, MODEL_WALL_01_ID, glm::vec3(2.0f, -10.0f, -21.5f));
    addModel("second_truck"_hs, MODEL_TRUCK_ID, glm::vec3(4.3f, 0.0f, -8.0f));

    auto& camera = _entityRegistry.emplace<Camera>(_entityRegistry.create());

    camera.active        = true;
    camera.movementSpeed = 2.0f;
    camera.rotationSpeed = 0.001f;
    camera.position      = { -3.0f, 0.0f, 0.0f };
    camera.yaw           = 0.0f;
    camera.pitch         = 0.0f;
    camera.nearPlane     = 0.01f;
    camera.farPlane      = 1000.0f;
    camera.fov           = glm::radians(60.0f);
}

void Application::uninitialize() {
    _serviceManager.destroy();
    _entityRegistry.clear();
}

void Application::saveState() {
    auto states = _serviceManager.saveState();
    auto result = makeSnapshot(_entityRegistry, states, SnapshotFormat::Capnproto);

    auto path = (std::filesystem::path(gerium_file_get_app_dir()) / "snapshot.bin").string();

    gerium_file_t file;
    check(gerium_file_create(path.c_str(), 0, &file));
    deferred(gerium_file_destroy(file));

    gerium_file_write(file, (gerium_cdata_t) result.data(), result.size());
}

void Application::loadState() {
    auto path = (std::filesystem::path(gerium_file_get_app_dir()) / "snapshot.bin").string();

    if (gerium_file_exists_file(path.c_str())) {
        gerium_file_t file;
        check(gerium_file_open(path.c_str(), true, &file));
        deferred(gerium_file_destroy(file));

        std::vector<gerium_uint8_t> data(gerium_file_get_size(file));
        gerium_file_read(file, (gerium_data_t) data.data(), data.size());

        std::map<hashed_string_owner, std::vector<uint8_t>> states;
        loadSnapshot(_entityRegistry, states, SnapshotFormat::Capnproto, data);
        _serviceManager.restoreState(states);
    }
}

void Application::frame(gerium_uint64_t elapsedMs) {
    _dispatcher.update();
    _serviceManager.update(elapsedMs);

    auto& settings = entityRegistry().ctx().get<Settings>();
    if (settings.state != Settings::None) {
        if (settings.state == Settings::Save) {
            saveState();
        } else {
            loadState();
        }
        settings.state = Settings::None;
    }
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
            break;
        default:
            break;
    }
}

gerium_bool_t Application::frame(gerium_application_t /* application */,
                                 gerium_data_t data,
                                 gerium_uint64_t elapsedMs) {
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

gerium_bool_t Application::state(gerium_application_t /* application */,
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
