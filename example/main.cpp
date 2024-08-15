#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Scene.hpp"

static gerium_application_t application = nullptr;
static gerium_logger_t logger           = nullptr;
static gerium_renderer_t renderer       = nullptr;
static gerium_frame_graph_t frameGraph  = nullptr;
static gerium_profiler_t profiler       = nullptr;

static gerium_technique_h baseTechnique    = {};
static gerium_technique_h presentTechnique = {};

static gerium_descriptor_set_h presentDescriptorSet = {};

static Scene scene{};
static std::shared_ptr<Camera> camera{};

gerium_bool_t simpleRender(gerium_frame_graph_t frame_graph,
                           gerium_renderer_t renderer,
                           gerium_command_buffer_t command_buffer,
                           gerium_uint32_t worker,
                           gerium_uint32_t total_workers,
                           gerium_data_t data) {
    auto models = scene.getComponents<Model>();

    gerium_command_buffer_bind_technique(command_buffer, baseTechnique);
    gerium_command_buffer_bind_descriptor_set(command_buffer, camera->getDecriptorSet(), 0);

    for (auto model : models) {
        for (auto& mesh : model->meshes()) {
            gerium_command_buffer_bind_descriptor_set(command_buffer, mesh.getMaterial().getDecriptorSet(), 1);
            gerium_command_buffer_bind_vertex_buffer(command_buffer, mesh.getPositions(), 0, mesh.getPositionsOffset());
            gerium_command_buffer_bind_vertex_buffer(command_buffer, mesh.getTexcoords(), 1, mesh.getTexcoordsOffset());
            gerium_command_buffer_bind_index_buffer(
                command_buffer, mesh.getIndices(), mesh.getIndicesOffset(), mesh.getIndexType());
            gerium_command_buffer_draw_indexed(command_buffer, 0, mesh.getPrimitiveCount(), 0, 0, 1);
        }
    }
    return 1;
}

gerium_bool_t presentRender(gerium_frame_graph_t frame_graph,
                            gerium_renderer_t renderer,
                            gerium_command_buffer_t command_buffer,
                            gerium_uint32_t worker,
                            gerium_uint32_t total_workers,
                            gerium_data_t data) {
    gerium_command_buffer_bind_technique(command_buffer, presentTechnique);
    gerium_command_buffer_bind_descriptor_set(command_buffer, presentDescriptorSet, 0);
    gerium_command_buffer_draw(command_buffer, 0, 3, 0, 1);
    gerium_command_buffer_draw_profiler(command_buffer, nullptr);
    return 1;
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
        gerium_renderer_set_profiler_enable(renderer, 1);

        check(gerium_frame_graph_create(renderer, &frameGraph));
        check(gerium_profiler_create(renderer, &profiler));

        check(gerium_renderer_create_descriptor_set(renderer, &presentDescriptorSet));
        gerium_renderer_bind_resource(renderer, presentDescriptorSet, 0, "color");

        gerium_render_pass_t presentPass{ 0, 0, presentRender };
        gerium_frame_graph_add_pass(frameGraph, "present_pass", &presentPass, nullptr);

        gerium_render_pass_t simplePass{ 0, 0, simpleRender };
        gerium_frame_graph_add_pass(frameGraph, "simple_pass", &simplePass, nullptr);

        gerium_resource_input_t presentInputs[] = {
            { GERIUM_RESOURCE_TYPE_TEXTURE, "color" },
        };
        check(gerium_frame_graph_add_node(
            frameGraph, "present_pass", std::size(presentInputs), presentInputs, 0, nullptr));

        gerium_resource_output_t simpleOutputs[2]{};
        simpleOutputs[0].type             = GERIUM_RESOURCE_TYPE_ATTACHMENT;
        simpleOutputs[0].name             = "color";
        simpleOutputs[0].format           = GERIUM_FORMAT_R8G8B8A8_UNORM;
        simpleOutputs[0].auto_scale       = 1.0f;
        simpleOutputs[0].render_pass_op   = GERIUM_RENDER_PASS_OP_CLEAR;
        simpleOutputs[0].color_write_mask = GERIUM_COLOR_COMPONENT_R_BIT | GERIUM_COLOR_COMPONENT_G_BIT |
                                            GERIUM_COLOR_COMPONENT_B_BIT | GERIUM_COLOR_COMPONENT_A_BIT;

        simpleOutputs[1].type           = GERIUM_RESOURCE_TYPE_ATTACHMENT;
        simpleOutputs[1].name           = "depth";
        simpleOutputs[1].format         = GERIUM_FORMAT_D32_SFLOAT;
        simpleOutputs[1].auto_scale     = 1.0f;
        simpleOutputs[1].render_pass_op = GERIUM_RENDER_PASS_OP_CLEAR;

        simpleOutputs[0].clear_color_attachment         = { 1.0f, 0.0f, 1.0f, 1.0f };
        simpleOutputs[1].clear_depth_stencil_attachment = { 1.0f, 0 };

        check(gerium_frame_graph_add_node(
            frameGraph, "simple_pass", 0, nullptr, std::size(simpleOutputs), simpleOutputs));

        check(gerium_frame_graph_compile(frameGraph));

        gerium_vertex_attribute_t vertexAttributes[] = {
            { 0, 0, 0, GERIUM_FORMAT_R32G32B32_SFLOAT },
            { 1, 1, 0, GERIUM_FORMAT_R32G32_SFLOAT    }
        };

        gerium_vertex_binding_t vertexBindings[] = {
            { 0, 12, GERIUM_VERTEX_RATE_PER_VERTEX },
            { 1, 8,  GERIUM_VERTEX_RATE_PER_VERTEX }
        };

        gerium_shader_t baseShaders[2]{};
        baseShaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
        baseShaders[0].lang = GERIUM_SHADER_LANGUAGE_GLSL;
        baseShaders[0].name = "base.vert.glsl";
        baseShaders[0].data = "#version 450\n"
                              "\n"
                              "layout(location = 0) in vec3 position;\n"
                              "layout(location = 1) in vec2 texcoord;\n"
                              "\n"
                              "layout(location = 0) out vec2 outTexcoord;\n"
                              "\n"
                              "layout(binding = 0, set = 0) uniform SceneData {\n"
                              "    mat4 viewProjection;\n"
                              "    vec4 eye;\n"
                              "} scene;\n"
                              "\n"
                              "layout(binding = 0, set = 1) uniform MeshData {\n"
                              "    mat4 world;\n"
                              "    mat4 inverseWorld;\n"
                              "} mesh;\n"
                              "\n"
                              "void main() {\n"
                              "    gl_Position = scene.viewProjection * mesh.world * vec4(position, 1.0);\n"
                              "    outTexcoord = texcoord;\n"
                              "}\n";
        baseShaders[0].size = strlen((const char*) baseShaders[0].data);

        baseShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
        baseShaders[1].lang = GERIUM_SHADER_LANGUAGE_GLSL;
        baseShaders[1].name = "base.frag.glsl";
        baseShaders[1].data = "#version 450\n"
                              "\n"
                              "layout(location = 0) in vec2 inTexcoord;\n"
                              "\n"
                              "layout(location = 0) out vec4 outColor;\n"
                              "\n"
                              "void main() {\n"
                              "    outColor = vec4(inTexcoord, 0.0, 1.0);\n"
                              "}\n";
        baseShaders[1].size = strlen((const char*) baseShaders[1].data);

        gerium_color_blend_state_t colorBlend{};
        gerium_depth_stencil_state_t depthStencil{};
        depthStencil.depth_test_enable        = 1;
        depthStencil.depth_write_enable       = 1;
        depthStencil.depth_bounds_test_enable = 0;
        depthStencil.stencil_test_enable      = 0;
        depthStencil.depth_compare_op         = GERIUM_COMPARE_OP_LESS_OR_EQUAL;

        gerium_rasterization_state_t rasterization{};
        rasterization.polygon_mode               = GERIUM_POLYGON_MODE_FILL;
        rasterization.cull_mode                  = GERIUM_CULL_MODE_BACK;
        rasterization.front_face                 = GERIUM_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization.depth_clamp_enable         = 0;
        rasterization.depth_bias_enable          = 0;
        rasterization.depth_bias_constant_factor = 0.0f;
        rasterization.depth_bias_clamp           = 0.0f;
        rasterization.depth_bias_slope_factor    = 0.0f;
        rasterization.line_width                 = 1.0f;

        gerium_pipeline_t basePipelines[1]{};
        basePipelines[0].render_pass            = "simple_pass";
        basePipelines[0].rasterization          = &rasterization;
        basePipelines[0].depth_stencil          = &depthStencil;
        basePipelines[0].color_blend            = &colorBlend;
        basePipelines[0].vertex_attribute_count = std::size(vertexAttributes);
        basePipelines[0].vertex_attributes      = vertexAttributes;
        basePipelines[0].vertex_binding_count   = std::size(vertexBindings);
        basePipelines[0].vertex_bindings        = vertexBindings;
        basePipelines[0].shader_count           = std::size(baseShaders);
        basePipelines[0].shaders                = baseShaders;

        check(gerium_renderer_create_technique(
            renderer, frameGraph, "base", std::size(basePipelines), basePipelines, &baseTechnique));

        gerium_shader_t presentShaders[2]{};
        presentShaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
        presentShaders[0].lang = GERIUM_SHADER_LANGUAGE_HLSL;
        presentShaders[0].name = "present.vert.hlsl";
        // presentShaders[0].data = "#version 450\n"
        //                             "\n"
        //                             "layout(location = 0) out vec2 vTexCoord;"
        //                             "\n"
        //                             "void main() {\n"
        //                             "    vTexCoord.xy = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);\n"
        //                             "    gl_Position = vec4(vTexCoord.xy * 2.0f - 1.0f, 0.0f, 1.0f);\n"
        //                             "    gl_Position.y = -gl_Position.y;\n"
        //                             "}\n";
        presentShaders[0].data = "struct Output {\n"
                                 "    float4 pos : SV_POSITION;\n"
                                 "    float2 uv  : TEXCOORD0;\n"
                                 "};\n"
                                 "\n"
                                 "Output main(uint id : SV_VertexID) {\n"
                                 "    Output output;\n"
                                 "    output.uv.x = float((id << 1) & 2);\n"
                                 "    output.uv.y = float(id & 2);\n"
                                 "    output.pos.xy = output.uv.xy * 2.0f - 1.0f;\n"
                                 "    output.pos.z = 0.0f;\n"
                                 "    output.pos.w = 1.0f;\n"
                                 "    output.pos.y = -output.pos.y;\n"
                                 "    return output;\n"
                                 "}\n";
        presentShaders[0].size = strlen((const char*) presentShaders[0].data);

        presentShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
        presentShaders[1].lang = GERIUM_SHADER_LANGUAGE_GLSL;
        presentShaders[1].name = "present.frag.glsl";
        presentShaders[1].data = "#version 450\n"
                                 "\n"
                                 "layout(location = 0) in vec2 vTexCoord;\n"

                                 "layout(binding = 0) uniform sampler2D texColor;\n"

                                 "layout(location = 0) out vec4 outColor;\n"
                                 "\n"
                                 "void main() {\n"
                                 "    outColor = texture(texColor, vTexCoord);\n"
                                 "}\n";
        presentShaders[1].size = strlen((const char*) presentShaders[1].data);

        gerium_depth_stencil_state_t depthStencilEmpty{};
        gerium_rasterization_state_t rasterizationEmpty{};
        gerium_pipeline_t presentPipelines[1]{};
        presentPipelines[0].render_pass   = "present_pass";
        presentPipelines[0].rasterization = &rasterizationEmpty;
        presentPipelines[0].depth_stencil = &depthStencilEmpty;
        presentPipelines[0].color_blend   = &colorBlend;
        presentPipelines[0].shader_count  = std::size(presentShaders);
        presentPipelines[0].shaders       = presentShaders;

        check(gerium_renderer_create_technique(
            renderer, frameGraph, "present", std::size(presentPipelines), presentPipelines, &presentTechnique));

        std::filesystem::path appDir = gerium_file_get_app_dir();

        auto sponzaDir       = appDir / "assets" / "models" / "sponza" / "Sponza.gltf";
        auto flightHelmetDir = appDir / "assets" / "models" / "flight-helmet" / "FlightHelmet.gltf";

        auto modelSponza       = Model::loadGlTF(renderer, sponzaDir.string().c_str());
        auto modelFlightHelmet = Model::loadGlTF(renderer, flightHelmetDir.string().c_str());

        auto defaultTransform = Transform{ glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), true };
        auto sponzaTransform  = Transform{ glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.0008f, 0.0008f, 0.0008f)),
                                          glm::identity<glm::mat4>(),
                                          true };
        auto flightHelmetTransform =
            Transform{ glm::rotate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.125f, 0.125f, 0.125f)),
                                   glm::radians(-90.0f),
                                   glm::vec3(0.0f, 1.0f, 0.0f)),
                       glm::identity<glm::mat4>(),
                       true };

        auto root         = scene.root();
        auto sponza       = scene.addNode(root);
        auto flightHelmet = scene.addNode(root);

        scene.addComponentToNode(root, defaultTransform);
        scene.addComponentToNode(sponza, sponzaTransform);
        scene.addComponentToNode(sponza, modelSponza);
        scene.addComponentToNode(flightHelmet, flightHelmetTransform);
        scene.addComponentToNode(flightHelmet, modelFlightHelmet);

        camera = std::make_shared<Camera>(application, renderer);

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
    if (renderer) {
        camera = nullptr;
        scene.clear();
        gerium_renderer_destroy_technique(renderer, presentTechnique);
        gerium_renderer_destroy_technique(renderer, baseTechnique);
        gerium_renderer_destroy_descriptor_set(renderer, presentDescriptorSet);
    }

    gerium_profiler_destroy(profiler);
    gerium_frame_graph_destroy(frameGraph);
    gerium_renderer_destroy(renderer);
}

gerium_bool_t frame(gerium_application_t application, gerium_data_t data, gerium_float32_t elapsed) {
    bool swapFullscreen = false;
    bool showCursor     = gerium_application_is_show_cursor(application);

    auto move = 1.0f;
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_SHIFT_LEFT) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_SHIFT_RIGHT)) {
        move *= 2.0f;
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_CONTROL_LEFT) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_CONTROL_RIGHT)) {
        move /= 2.0f;
    }

    gerium_event_t event;
    while (gerium_application_poll_events(application, &event)) {
        if (event.type == GERIUM_EVENT_TYPE_KEYBOARD) {
            if (event.keyboard.scancode == GERIUM_SCANCODE_ENTER && event.keyboard.state == GERIUM_KEY_STATE_RELEASED &&
                (event.keyboard.modifiers & GERIUM_KEY_MOD_LALT)) {
                swapFullscreen = true;
            } else if (event.keyboard.scancode == GERIUM_SCANCODE_ESCAPE &&
                       event.keyboard.state == GERIUM_KEY_STATE_RELEASED) {
                showCursor = true;
            }
        } else if (event.type == GERIUM_EVENT_TYPE_MOUSE) {
            constexpr auto buttonsDown = GERIUM_MOUSE_BUTTON_LEFT_DOWN | GERIUM_MOUSE_BUTTON_RIGHT_DOWN |
                                         GERIUM_MOUSE_BUTTON_MIDDLE_DOWN | GERIUM_MOUSE_BUTTON_4_DOWN |
                                         GERIUM_MOUSE_BUTTON_5_DOWN;
            if (event.mouse.buttons & buttonsDown) {
                showCursor = false;
            }
            if (!gerium_application_is_show_cursor(application)) {
                gerium_float32_t pitch = event.mouse.raw_delta_y * -1.0f;
                gerium_float32_t yaw   = event.mouse.raw_delta_x * 1.0f;
                camera->rotate(pitch, yaw, elapsed);
                camera->zoom(event.mouse.wheel_vertical * move * -0.01f, elapsed);
            }
        }
    }

    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_A) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_ARROW_LEFT)) {
        camera->move(Camera::Right, -move, elapsed);
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_D) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_ARROW_RIGHT)) {
        camera->move(Camera::Right, move, elapsed);
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_W) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_ARROW_UP)) {
        camera->move(Camera::Forward, move, elapsed);
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_S) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_ARROW_DOWN)) {
        camera->move(Camera::Forward, -move, elapsed);
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_SPACE) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_PAGE_UP)) {
        camera->move(Camera::Up, move, elapsed);
    }
    if (gerium_application_is_press_scancode(application, GERIUM_SCANCODE_C) ||
        gerium_application_is_press_scancode(application, GERIUM_SCANCODE_PAGE_DOWN)) {
        camera->move(Camera::Up, -move, elapsed);
    }

    if (swapFullscreen) {
        gerium_application_fullscreen(application, !gerium_application_is_fullscreen(application), 0, nullptr);
    }

    if (gerium_application_get_platform(application) != GERIUM_RUNTIME_PLATFORM_ANDROID) {
        gerium_application_show_cursor(application, showCursor);
    }

    if (gerium_renderer_new_frame(renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return 1;
    }

    scene.update();
    camera->update();

    gerium_renderer_render(renderer, frameGraph);
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
            gerium_application_show_cursor(application, 1);
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_GOT_FOCUS");
            break;
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
            gerium_application_show_cursor(application, 1);
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_LOST_FOCUS");
            break;
        case GERIUM_APPLICATION_STATE_VISIBLE:
            gerium_application_show_cursor(application, 1);
            gerium_logger_print(logger, GERIUM_LOGGER_LEVEL_DEBUG, "GERIUM_APPLICATION_STATE_VISIBLE");
            break;
        case GERIUM_APPLICATION_STATE_INVISIBLE:
            gerium_application_show_cursor(application, 1);
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

#ifdef GERIUM_PLATFORM_WINDOWS
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main() {
#endif
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
