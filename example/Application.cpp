#include "Application.hpp"

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
}

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto ds = application()->resourceManager().createDescriptorSet("");
    gerium_renderer_bind_resource(renderer, ds, 0, "color");

    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, 0);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);
    gerium_command_buffer_draw_profiler(commandBuffer, nullptr);

    if (ImGui::Begin("Info")) {
        ImGui::LabelText(application()->meshShaderSupported() ? "hardware" : "software", "Meshlets");
    }

    ImGui::End();
}

Application::Application() {
    check(gerium_logger_create("example", &_logger));
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

void Application::run(gerium_utf8_t title, gerium_uint32_t width, gerium_uint32_t height) {
    try {
        check(gerium_application_create(title, width, height, &_application));
        gerium_application_set_background_wait(_application, true);
        gerium_application_set_frame_func(_application, frame, (gerium_data_t) this);
        gerium_application_set_state_func(_application, state, (gerium_data_t) this);
        check(gerium_application_run(_application));
        if (_error) {
            std::rethrow_exception(_error);
        }
    } catch (const std::exception& exc) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, exc.what());
    } catch (...) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_FATAL, "unknown error");
    }
}

void Application::addPass(RenderPass& renderPass) {
    renderPass.setApplication(this);
    _renderPasses.push_back(&renderPass);

    gerium_render_pass_t pass{ prepare, resize, render };
    gerium_frame_graph_add_pass(_frameGraph, renderPass.name().c_str(), &pass, &renderPass);
}

float get_random_value(float min, float max) {
    float rnd = (float) rand() / (float) RAND_MAX;

    rnd = (max - min) * rnd + min;

    return rnd;
}

void Application::createScene() {
    std::filesystem::path appDir = gerium_file_get_app_dir();
}

void Application::initialize() {
    constexpr auto debug =
#ifdef NDEBUG
        false;
#else
        true;
#endif

    check(gerium_renderer_create(_application,
                                 GERIUM_FEATURE_BINDLESS_BIT | GERIUM_FEATURE_MESH_SHADER_BIT |
                                     GERIUM_FEATURE_8_BIT_STORAGE_BIT | GERIUM_FEATURE_16_BIT_STORAGE_BIT,
                                 GERIUM_VERSION_ENCODE(1, 0, 0),
                                 debug,
                                 &_renderer));
    gerium_renderer_set_profiler_enable(_renderer, true);

    _bindlessSupported   = gerium_renderer_get_enabled_features(_renderer) & GERIUM_FEATURE_BINDLESS_BIT;
    _meshShaderSupported = gerium_renderer_get_enabled_features(_renderer) & GERIUM_FEATURE_MESH_SHADER_BIT;

    check(gerium_profiler_create(_renderer, &_profiler));
    check(gerium_frame_graph_create(_renderer, &_frameGraph));

    _asyncLoader.create(_application, _renderer);
    _resourceManager.create(_asyncLoader, _frameGraph);

    addPass(_presentPass);
    addPass(_gbufferPass);

    std::filesystem::path appDir = gerium_file_get_app_dir();
    _resourceManager.loadFrameGraph((appDir / "frame-graphs" / "main.yaml").string());
    _baseTechnique = _resourceManager.loadTechnique((appDir / "techniques" / "base.yaml").string());

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }

    _camera = Camera(_application, _resourceManager);

    createScene();
}

void Application::uninitialize() {
    if (_renderer) {
        _asyncLoader.destroy();

        _baseTechnique = nullptr;

        for (auto it = _renderPasses.rbegin(); it != _renderPasses.rend(); ++it) {
            (*it)->uninitialize(_frameGraph, _renderer);
        }

        _resourceManager.destroy();

        if (_frameGraph) {
            gerium_frame_graph_destroy(_frameGraph);
            _frameGraph = nullptr;
        }
        if (_profiler) {
            gerium_profiler_destroy(_profiler);
            _profiler = nullptr;
        }
        gerium_renderer_destroy(_renderer);
        _renderer = nullptr;
    }
}

void Application::pollInput(gerium_uint64_t elapsedMs) {
    bool swapFullscreen = false;
    bool showCursor     = gerium_application_is_show_cursor(_application);
    bool invY           = gerium_application_get_platform(_application) == GERIUM_RUNTIME_PLATFORM_MAC_OS;

    auto move = 1.0f;
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_SHIFT_LEFT) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_SHIFT_RIGHT)) {
        move *= 2.0f;
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_CONTROL_LEFT) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_CONTROL_RIGHT)) {
        move /= 2.0f;
    }

    gerium_event_t event;
    gerium_float32_t pitch = 0.0f;
    gerium_float32_t yaw   = 0.0f;
    gerium_float32_t zoom  = 0.0f;
    while (gerium_application_poll_events(_application, &event)) {
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
            if (!gerium_application_is_show_cursor(_application) ||
                gerium_application_get_platform(_application) == GERIUM_RUNTIME_PLATFORM_ANDROID) {
                const auto delta = 1.0f;
                pitch += event.mouse.raw_delta_y * (invY ? delta : -delta);
                yaw += event.mouse.raw_delta_x * -delta;
                zoom += event.mouse.wheel_vertical * move * -0.1f;
            }
        }
    }

    auto camera = getCamera();
    camera->rotate(pitch, yaw, 1.0f);
    camera->zoom(zoom, 1.0f);

    auto elapsed = (gerium_float32_t) elapsedMs;
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_A) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_LEFT)) {
        camera->move(Camera::Right, move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_D) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_RIGHT)) {
        camera->move(Camera::Right, -move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_W) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_UP)) {
        camera->move(Camera::Forward, move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_S) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_DOWN)) {
        camera->move(Camera::Forward, -move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_SPACE) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_PAGE_UP)) {
        camera->move(Camera::Up, move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_C) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_PAGE_DOWN)) {
        camera->move(Camera::Up, -move, elapsed);
    }

    if (swapFullscreen) {
        gerium_application_fullscreen(_application, !gerium_application_is_fullscreen(_application), 0, nullptr);
    }

    if (gerium_application_get_platform(_application) != GERIUM_RUNTIME_PLATFORM_ANDROID) {
        gerium_application_show_cursor(_application, showCursor);
    }
}

void Application::frame(gerium_uint64_t elapsedMs) {
    pollInput(elapsedMs);

    if (gerium_renderer_new_frame(_renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return;
    }

    _prevWidth  = _width;
    _prevHeight = _height;
    gerium_application_get_size(_application, &_width, &_height);

    if (_prevWidth != _width || _prevHeight != _height) {
        _invWidth  = 1.0f / _width;
        _invHeight = 1.0f / _height;
    }

    getCamera()->update();

    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);
}

void Application::state(gerium_application_state_t state) {
    const auto stateStr = magic_enum::enum_name(state);
    gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_DEBUG, stateStr.data());

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
            gerium_application_show_cursor(_application, true);
            break;
        default:
            break;
    }
}

gerium_bool_t Application::frame(gerium_application_t application, gerium_data_t data, gerium_uint64_t elapsedMs) {
    auto app = (Application*) data;
    return app->cppCall([app, elapsedMs]() {
        app->frame(elapsedMs);
    });
}

gerium_bool_t Application::state(gerium_application_t application,
                                 gerium_data_t data,
                                 gerium_application_state_t state) {
    auto app = (Application*) data;
    return app->cppCall([app, state]() {
        app->state(state);
    });
}

gerium_uint32_t Application::prepare(gerium_frame_graph_t frameGraph,
                                     gerium_renderer_t renderer,
                                     gerium_uint32_t maxWorkers,
                                     gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCallInt([renderPass, frameGraph, renderer, maxWorkers]() {
        return renderPass->prepare(frameGraph, renderer, maxWorkers);
    });
}

gerium_bool_t Application::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCall([renderPass, frameGraph, renderer]() {
        renderPass->resize(frameGraph, renderer);
    });
}

gerium_bool_t Application::render(gerium_frame_graph_t frameGraph,
                                  gerium_renderer_t renderer,
                                  gerium_command_buffer_t commandBuffer,
                                  gerium_uint32_t worker,
                                  gerium_uint32_t totalWorkers,
                                  gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->application()->cppCall(
        [renderPass, frameGraph, renderer, commandBuffer, worker, totalWorkers]() {
        renderPass->render(frameGraph, renderer, commandBuffer, worker, totalWorkers);
    });
}
