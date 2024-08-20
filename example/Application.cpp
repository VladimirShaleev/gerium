#include "Application.hpp"

void SimplePass::render(gerium_frame_graph_t frameGraph,
                        gerium_renderer_t renderer,
                        gerium_command_buffer_t commandBuffer,
                        gerium_uint32_t worker,
                        gerium_uint32_t totalWorkers) {
    auto& manager  = getApplication()->resourceManager();
    auto& scene    = getApplication()->scene();
    auto camera    = scene.getAnyComponentNode<Camera>();
    auto models    = scene.getComponents<Model>();
    auto technique = manager.loadTechnique("base");

    gerium_command_buffer_bind_technique(commandBuffer, technique);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), 0);

    for (auto model : models) {
        for (auto& mesh : model->meshes()) {
            gerium_command_buffer_bind_descriptor_set(commandBuffer, mesh.getMaterial().getDecriptorSet(), 1);
            gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh.getPositions(), 0, mesh.getPositionsOffset());
            gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh.getTexcoords(), 1, mesh.getTexcoordsOffset());
            gerium_command_buffer_bind_index_buffer(
                commandBuffer, mesh.getIndices(), mesh.getIndicesOffset(), mesh.getIndexType());
            gerium_command_buffer_draw_indexed(commandBuffer, 0, mesh.getPrimitiveCount(), 0, 0, 1);
        }
    }
}

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    gerium_command_buffer_bind_technique(commandBuffer, _technique);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, 0);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);
    gerium_command_buffer_draw_profiler(commandBuffer, nullptr);
}

void PresentPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    std::filesystem::path relative("shaders");
    const auto presentVert = (relative / "present.vert.hlsl").string();
    const auto presentFrag = (relative / "present.frag.glsl").string();

    gerium_shader_t shaders[2]{};
    shaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
    shaders[0].lang = GERIUM_SHADER_LANGUAGE_HLSL;
    shaders[0].name = presentVert.c_str();
    shaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
    shaders[1].lang = GERIUM_SHADER_LANGUAGE_GLSL;
    shaders[1].name = presentFrag.c_str();

    gerium_color_blend_state_t colorBlend{};
    gerium_depth_stencil_state_t depthStencilEmpty{};
    gerium_rasterization_state_t rasterizationEmpty{};
    rasterizationEmpty.line_width = 1.0f;
    std::vector<gerium_pipeline_t> pipelines;
    pipelines.resize(1);
    pipelines[0].render_pass   = name().c_str();
    pipelines[0].rasterization = &rasterizationEmpty;
    pipelines[0].depth_stencil = &depthStencilEmpty;
    pipelines[0].color_blend   = &colorBlend;
    pipelines[0].shader_count  = std::size(shaders);
    pipelines[0].shaders       = shaders;

    _technique     = getApplication()->resourceManager().createTechnique("present", pipelines);
    _descriptorSet = getApplication()->resourceManager().createDescriptorSet();

    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "color");
}

void PresentPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
    _technique     = nullptr;
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

        gerium_display_info_t displays[10];
        gerium_uint32_t displayCount = std::size(displays);
        check(gerium_application_get_display_info(_application, &displayCount, displays));

        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_INFO, "Avaiable monitors:");
        for (gerium_uint32_t i = 0; i < displayCount; ++i) {
            std::string log = "\t";
            log += displays[i].device_name;
            gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_INFO, log.c_str());
        }

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
    renderPass.initialize(_frameGraph, _renderer);
    _renderPasses.push_back(&renderPass);

    gerium_render_pass_t pass{ prepare, resize, render };
    gerium_frame_graph_add_pass(_frameGraph, renderPass.name().c_str(), &pass, &renderPass);
}

void Application::createFrameGraph() {
    gerium_resource_input_t presentInputs[] = {
        { GERIUM_RESOURCE_TYPE_TEXTURE, "color" },
    };

    gerium_resource_output_t simpleOutputs[2]{};
    simpleOutputs[0].type             = GERIUM_RESOURCE_TYPE_ATTACHMENT;
    simpleOutputs[0].name             = "color";
    simpleOutputs[0].format           = GERIUM_FORMAT_R8G8B8A8_UNORM;
    simpleOutputs[0].auto_scale       = 1.0f;
    simpleOutputs[0].render_pass_op   = GERIUM_RENDER_PASS_OP_CLEAR;
    simpleOutputs[0].color_write_mask = GERIUM_COLOR_COMPONENT_R_BIT | GERIUM_COLOR_COMPONENT_G_BIT |
                                        GERIUM_COLOR_COMPONENT_B_BIT | GERIUM_COLOR_COMPONENT_A_BIT;
    simpleOutputs[0].clear_color_attachment = { 1.0f, 0.0f, 1.0f, 1.0f };

    simpleOutputs[1].type                           = GERIUM_RESOURCE_TYPE_ATTACHMENT;
    simpleOutputs[1].name                           = "depth";
    simpleOutputs[1].format                         = GERIUM_FORMAT_D32_SFLOAT;
    simpleOutputs[1].auto_scale                     = 1.0f;
    simpleOutputs[1].render_pass_op                 = GERIUM_RENDER_PASS_OP_CLEAR;
    simpleOutputs[1].clear_depth_stencil_attachment = { 1.0f, 0 };

    check(gerium_frame_graph_add_node(
        _frameGraph, _presentPass.name().c_str(), std::size(presentInputs), presentInputs, 0, nullptr));
    check(gerium_frame_graph_add_node(
        _frameGraph, _simplePass.name().c_str(), 0, nullptr, std::size(simpleOutputs), simpleOutputs));

    check(gerium_frame_graph_compile(_frameGraph));
}

void Application::createBaseTechnique() {
    gerium_vertex_attribute_t vertexAttributes[] = {
        { 0, 0, 0, GERIUM_FORMAT_R32G32B32_SFLOAT },
        { 1, 1, 0, GERIUM_FORMAT_R32G32_SFLOAT    }
    };

    gerium_vertex_binding_t vertexBindings[] = {
        { 0, 12, GERIUM_VERTEX_RATE_PER_VERTEX },
        { 1, 8,  GERIUM_VERTEX_RATE_PER_VERTEX }
    };

    std::filesystem::path relative("shaders");
    const auto baseVert = (relative / "base.vert.glsl").string();
    const auto baseFrag = (relative / "base.frag.glsl").string();

    gerium_shader_t baseShaders[2]{};
    baseShaders[0].type = GERIUM_SHADER_TYPE_VERTEX;
    baseShaders[0].lang = GERIUM_SHADER_LANGUAGE_GLSL;
    baseShaders[0].name = baseVert.c_str();
    baseShaders[1].type = GERIUM_SHADER_TYPE_FRAGMENT;
    baseShaders[1].lang = GERIUM_SHADER_LANGUAGE_GLSL;
    baseShaders[1].name = baseFrag.c_str();

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

    std::vector<gerium_pipeline_t> basePipelines;
    basePipelines.resize(1);
    basePipelines[0].render_pass            = _simplePass.name().c_str();
    basePipelines[0].rasterization          = &rasterization;
    basePipelines[0].depth_stencil          = &depthStencil;
    basePipelines[0].color_blend            = &colorBlend;
    basePipelines[0].vertex_attribute_count = std::size(vertexAttributes);
    basePipelines[0].vertex_attributes      = vertexAttributes;
    basePipelines[0].vertex_binding_count   = std::size(vertexBindings);
    basePipelines[0].vertex_bindings        = vertexBindings;
    basePipelines[0].shader_count           = std::size(baseShaders);
    basePipelines[0].shaders                = baseShaders;

    _baseTechnique = _resourceManager.createTechnique("base", basePipelines);
}

void Application::createScene() {
    std::filesystem::path appDir = gerium_file_get_app_dir();

    auto sponzaDir       = (appDir / "assets" / "models" / "sponza" / "Sponza.gltf").string();
    auto flightHelmetDir = (appDir / "assets" / "models" / "flight-helmet" / "FlightHelmet.gltf").string();

    auto modelSponza       = Model::loadGlTF(_renderer, _resourceManager, sponzaDir.c_str());
    auto modelFlightHelmet = Model::loadGlTF(_renderer, _resourceManager, flightHelmetDir.c_str());

    auto defaultTransform = Transform{ glm::identity<glm::mat4>(), glm::identity<glm::mat4>(), true };
    auto sponzaTransform  = Transform{ glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.15f, 0.15f, 0.15f)),
                                      glm::identity<glm::mat4>(),
                                      true };
    auto flightHelmetTransform =
        Transform{ glm::rotate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.125f, 0.125f, 0.125f)),
                               glm::radians(-90.0f),
                               glm::vec3(0.0f, 1.0f, 0.0f)),
                   glm::identity<glm::mat4>(),
                   true };

    auto root         = _scene.root();
    auto sponza       = _scene.addNode(root);
    auto flightHelmet = _scene.addNode(root);

    _scene.addComponentToNode(root, defaultTransform);
    _scene.addComponentToNode(root, Camera(_application, _resourceManager));
    _scene.addComponentToNode(sponza, sponzaTransform);
    _scene.addComponentToNode(sponza, modelSponza);
    _scene.addComponentToNode(flightHelmet, flightHelmetTransform);
    _scene.addComponentToNode(flightHelmet, modelFlightHelmet);
}

void Application::initialize() {
    constexpr auto debug =
#ifdef NDEBUG
        false;
#else
        true;
#endif

    check(gerium_renderer_create(_application, GERIUM_VERSION_ENCODE(1, 0, 0), debug, &_renderer));
    gerium_renderer_set_profiler_enable(_renderer, true);

    check(gerium_profiler_create(_renderer, &_profiler));
    check(gerium_frame_graph_create(_renderer, &_frameGraph));

    _asyncLoader.create(_application, _renderer);
    _resourceManager.create(_asyncLoader, _frameGraph);

    addPass(_presentPass);
    addPass(_simplePass);
    createFrameGraph();
    createBaseTechnique();
    createScene();
}

void Application::uninitialize() {
    if (_renderer) {
        _asyncLoader.destroy();
        _scene.clear();

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

void Application::pollInput(gerium_float32_t elapsed) {
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
                yaw += event.mouse.raw_delta_x * delta;
                zoom += event.mouse.wheel_vertical * move * -0.1f;
            }
        }
    }

    auto camera = _scene.getAnyComponentNode<Camera>();

    camera->rotate(pitch, yaw, 1.0f);
    camera->zoom(zoom, 1.0f);

    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_A) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_LEFT)) {
        camera->move(Camera::Right, -move, elapsed);
    }
    if (gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_D) ||
        gerium_application_is_press_scancode(_application, GERIUM_SCANCODE_ARROW_RIGHT)) {
        camera->move(Camera::Right, move, elapsed);
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

    if (gerium_renderer_new_frame(_renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return;
    }
}

void Application::frame(gerium_float32_t elapsed) {
    pollInput(elapsed);

    _resourceManager.update(elapsed);
    _scene.update();

    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);
}

void Application::state(gerium_application_state_t state) {
    const auto stateStr = stateToString(state);
    gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_DEBUG, stateStr.c_str());

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

gerium_bool_t Application::frame(gerium_application_t application, gerium_data_t data, gerium_float32_t elapsed) {
    auto app = (Application*) data;
    return app->cppCall([app, elapsed]() {
        app->frame(elapsed);
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

gerium_uint32_t Application::prepare(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->getApplication()->cppCallInt([renderPass, frameGraph, renderer]() {
        return renderPass->prepare(frameGraph, renderer);
    });
}

gerium_bool_t Application::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    return renderPass->getApplication()->cppCall([renderPass, frameGraph, renderer]() {
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
    return renderPass->getApplication()->cppCall(
        [renderPass, frameGraph, renderer, commandBuffer, worker, totalWorkers]() {
        renderPass->render(frameGraph, renderer, commandBuffer, worker, totalWorkers);
    });
}

std::string Application::stateToString(gerium_application_state_t state) noexcept {
    switch (state) {
        case GERIUM_APPLICATION_STATE_CREATE:
            return "GERIUM_APPLICATION_STATE_CREATE";
        case GERIUM_APPLICATION_STATE_DESTROY:
            return "GERIUM_APPLICATION_STATE_DESTROY";
        case GERIUM_APPLICATION_STATE_INITIALIZE:
            return "GERIUM_APPLICATION_STATE_INITIALIZE";
        case GERIUM_APPLICATION_STATE_UNINITIALIZE:
            return "GERIUM_APPLICATION_STATE_UNINITIALIZE";
        case GERIUM_APPLICATION_STATE_GOT_FOCUS:
            return "GERIUM_APPLICATION_STATE_GOT_FOCUS";
        case GERIUM_APPLICATION_STATE_LOST_FOCUS:
            return "GERIUM_APPLICATION_STATE_LOST_FOCUS";
        case GERIUM_APPLICATION_STATE_VISIBLE:
            return "GERIUM_APPLICATION_STATE_VISIBLE";
        case GERIUM_APPLICATION_STATE_INVISIBLE:
            return "GERIUM_APPLICATION_STATE_INVISIBLE";
        case GERIUM_APPLICATION_STATE_NORMAL:
            return "GERIUM_APPLICATION_STATE_NORMAL";
        case GERIUM_APPLICATION_STATE_MINIMIZE:
            return "GERIUM_APPLICATION_STATE_MINIMIZE";
        case GERIUM_APPLICATION_STATE_MAXIMIZE:
            return "GERIUM_APPLICATION_STATE_MAXIMIZE";
        case GERIUM_APPLICATION_STATE_FULLSCREEN:
            return "GERIUM_APPLICATION_STATE_FULLSCREEN";
        case GERIUM_APPLICATION_STATE_RESIZE:
            return "GERIUM_APPLICATION_STATE_RESIZE";
        case GERIUM_APPLICATION_STATE_RESIZED:
            return "GERIUM_APPLICATION_STATE_RESIZED";
        default:
            return "GERIUM_APPLICATION_STATE_UNKNOWN";
    }
}
