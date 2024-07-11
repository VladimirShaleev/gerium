#include <gerium/gerium.h>
#include <string>
#include <stdexcept>

static gerium_logger_t logger     = nullptr;
static gerium_renderer_t renderer = nullptr;
static gerium_profiler_t profiler = nullptr;

void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

bool initialize(gerium_application_t application) {
    try {
        constexpr auto debug =
#ifdef NDEBUG
            false;
#else
            true;
#endif

        check(gerium_renderer_create(application, GERIUM_VERSION_ENCODE(1, 0, 0), debug, &renderer));
        check(gerium_profiler_create(renderer, &profiler));

        gerium_texture_creation_t creation;
        creation.width   = 128;
        creation.height  = 128;
        creation.depth   = 1;
        creation.mipmaps = 1;
        creation.format  = GERIUM_FORMAT_R8G8B8A8_UINT;
        creation.type    = GERIUM_TEXTURE_TYPE_2D;
        creation.data    = nullptr;
        creation.name    = "texture";

        gerium_texture_h texture;
        check(gerium_renderer_create_texture(renderer, &creation, &texture));

        gerium_renderer_destroy_texture(renderer, texture);

        int i = 0;

    } catch (const std::runtime_error& exc) {
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
        return false;
    } catch (...) {
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
        return false;
    }
    return true;
}

void unitialize(gerium_application_t application) {
    gerium_profiler_destroy(profiler);
    gerium_renderer_destroy(renderer);
}

gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_float32_t elapsed) {
    gerium_renderer_new_frame(renderer);
    gerium_renderer_present(renderer);
    return 1;
}

gerium_bool_t state(gerium_application_t application, gerium_data_t data, gerium_application_state_t state) {
    switch (state) {
        case GERIUM_APPLICATION_STATE_CREATE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_CREATE");
            break;
        case GERIUM_APPLICATION_STATE_DESTROY:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_DESTROY");
            break;
        case GERIUM_APPLICATION_STATE_INITIALIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_INITIALIZE");
            return initialize(application) ? 1 : 0;
        case GERIUM_APPLICATION_STATE_UNINITIALIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_UNINITIALIZE");
            unitialize(application);
            break;
        case GERIUM_APPLICATION_STATE_GOT_FOCUS:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_GOT_FOCUS");
            break;
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_LOST_FOCUS");
            break;
        case GERIUM_APPLICATION_STATE_VISIBLE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_VISIBLE");
            break;
        case GERIUM_APPLICATION_STATE_INVISIBLE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_INVISIBLE");
            break;
        case GERIUM_APPLICATION_STATE_NORMAL:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_NORMAL");
            break;
        case GERIUM_APPLICATION_STATE_MINIMIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_MINIMIZE");
            break;
        case GERIUM_APPLICATION_STATE_MAXIMIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_MAXIMIZE");
            break;
        case GERIUM_APPLICATION_STATE_FULLSCREEN:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_FULLSCREEN");
            break;
        case GERIUM_APPLICATION_STATE_RESIZE:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_RESIZE");
            break;
        case GERIUM_APPLICATION_STATE_RESIZED:
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_RESIZED");
            break;
    }
    return 1;
}

int main() {
    gerium_application_t application = nullptr;

    try {
        check(gerium_logger_create("app", &logger));
        check(gerium_application_create("test", 800, 600, &application));
        gerium_application_set_background_wait(application, 1);

        gerium_display_info_t displays[10];
        gerium_uint32_t displayCount = 10;
        check(gerium_application_get_display_info(application, &displayCount, displays));

        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_INFO, "Avaiable monitors:");
        for (gerium_uint32_t i = 0; i < displayCount; ++i) {
            std::string log = "\t";
            log += displays[i].device_name;
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_INFO, log.c_str());
        }

        gerium_application_set_frame_func(application, frame, nullptr);
        gerium_application_set_state_func(application, state, nullptr);
        check(gerium_application_run(application));

    } catch (const std::runtime_error& exc) {
        if (logger) {
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
        }
    } catch (...) {
        if (logger) {
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
        }
    }
    gerium_application_destroy(application);
    gerium_logger_destroy(logger);
    return 0;
}
