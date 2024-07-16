#include <gerium/gerium.h>
#include <stdexcept>
#include <string>

static gerium_application_t application = nullptr;
static gerium_logger_t logger           = nullptr;
static gerium_renderer_t renderer       = nullptr;
static gerium_frame_graph_t frameGraph  = nullptr;
static gerium_profiler_t profiler       = nullptr;

void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS && result != GERIUM_RESULT_SKIP_FRAME) {
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
        check(gerium_frame_graph_create(renderer, &frameGraph));
        check(gerium_profiler_create(renderer, &profiler));

        gerium_render_pass_t gbufferPass{};
        gerium_render_pass_t transparentPass{};
        gerium_render_pass_t depthOfFieldPass{};
        gerium_render_pass_t lightingPass{};
        gerium_render_pass_t depthPrePass{};
        gerium_frame_graph_add_pass(frameGraph, "gbuffer_pass", &gbufferPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "transparent_pass", &transparentPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "depth_of_field_pass", &depthOfFieldPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "lighting_pass", &lightingPass, nullptr);
        gerium_frame_graph_add_pass(frameGraph, "depth_pre_pass", &depthPrePass, nullptr);

        gerium_resource_input_t gbufferInputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT, "depth" }
        };
        gerium_resource_output_t gbufferOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "color",                        0,
             GERIUM_FORMAT_R8G8B8A8_UNORM,      0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "normals",                      0,
             GERIUM_FORMAT_R16G16B16A16_SFLOAT, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "metallic_roughness_occlusion", 0,
             GERIUM_FORMAT_R8G8B8A8_UNORM,      0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "position",                     0,
             GERIUM_FORMAT_R16G16B16A16_SFLOAT, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR },
        };

        gerium_resource_input_t transparentInputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT, "lighting" },
            { GERIUM_RESOURCE_TYPE_ATTACHMENT, "depth"    }
        };
        gerium_resource_output_t transparentOutputs[] = {
            { GERIUM_RESOURCE_TYPE_REFERENCE, "lighting" }
        };

        gerium_resource_input_t dofInputs[] = {
            { GERIUM_RESOURCE_TYPE_REFERENCE, "lighting" },
            { GERIUM_RESOURCE_TYPE_REFERENCE, "depth"    },
        };
        gerium_resource_output_t dofOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "final", 0,
             GERIUM_FORMAT_R8G8B8A8_UNORM, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR }
        };

        gerium_resource_input_t lightingInputs[] = {
            { GERIUM_RESOURCE_TYPE_TEXTURE, "color"                        },
            { GERIUM_RESOURCE_TYPE_TEXTURE, "normals"                      },
            { GERIUM_RESOURCE_TYPE_TEXTURE, "metallic_roughness_occlusion" },
            { GERIUM_RESOURCE_TYPE_TEXTURE, "position"                     }
        };
        gerium_resource_output_t lightingOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "lighting", 0,
             GERIUM_FORMAT_R8G8B8A8_UNORM, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR }
        };

        gerium_resource_output_t depthOutputs[] = {
            { GERIUM_RESOURCE_TYPE_ATTACHMENT,
             "depth", 0,
             GERIUM_FORMAT_D32_SFLOAT, 0,
             0, GERIUM_RENDER_PASS_OPERATION_CLEAR }
        };

        check(gerium_frame_graph_add_node(frameGraph,
                                          "gbuffer_pass",
                                          std::size(gbufferInputs),
                                          gbufferInputs,
                                          std::size(gbufferOutputs),
                                          gbufferOutputs));
        check(gerium_frame_graph_add_node(frameGraph,
                                          "transparent_pass",
                                          std::size(transparentInputs),
                                          transparentInputs,
                                          std::size(transparentOutputs),
                                          transparentOutputs));
        check(gerium_frame_graph_add_node(
            frameGraph, "depth_of_field_pass", std::size(dofInputs), dofInputs, std::size(dofOutputs), dofOutputs));
        check(gerium_frame_graph_add_node(frameGraph,
                                          "lighting_pass",
                                          std::size(lightingInputs),
                                          lightingInputs,
                                          std::size(lightingOutputs),
                                          lightingOutputs));
        check(gerium_frame_graph_add_node(
            frameGraph, "depth_pre_pass", 0, nullptr, std::size(depthOutputs), depthOutputs));
        check(gerium_frame_graph_compile(frameGraph));

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
    gerium_frame_graph_destroy(frameGraph);
    gerium_renderer_destroy(renderer);
}

gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_float32_t elapsed) {
    if (gerium_renderer_new_frame(renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return 1;
    }

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
    check(gerium_logger_create("app", &logger));

    try {
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
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
    } catch (...) {
        gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
    }
    gerium_application_destroy(application);
    gerium_logger_destroy(logger);
    return 0;
}
