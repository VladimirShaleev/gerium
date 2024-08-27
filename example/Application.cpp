#include "Application.hpp"

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto& manager = getApplication()->resourceManager();
    auto& scene   = getApplication()->scene();
    auto camera   = scene.getActiveCamera();

    for (auto mesh : scene.visibleMeshes()) {
        if (mesh->getMaterial().getFlags() != DrawFlags::None &&
            mesh->getMaterial().getFlags() != DrawFlags::DoubleSided) {
            continue;
        }
        if (((gerium_buffer_h) mesh->getTangents()).unused == 65535) {
            continue;
        }
        gerium_command_buffer_bind_technique(commandBuffer, mesh->getMaterial().getTechnique());
        gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), 0);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, mesh->getMaterial().getDecriptorSet(), 1);
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh->getPositions(), 0, mesh->getPositionsOffset());
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh->getTexcoords(), 1, mesh->getTexcoordsOffset());
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh->getNormals(), 2, mesh->getNormalsOffset());
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh->getTangents(), 3, mesh->getTangentsOffset());
        gerium_command_buffer_bind_index_buffer(
            commandBuffer, mesh->getIndices(), mesh->getIndicesOffset(), mesh->getIndexType());
        gerium_command_buffer_draw_indexed(commandBuffer, 0, mesh->getPrimitiveCount(), 0, 0, 1);
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

    if (ImGui::Begin("Settings")) {
        ImGui::Checkbox("Draw bbox", &_drawBBox);
        ImGui::Checkbox("Camera 2", &_camera2);

        Camera* camera[2];
        gerium_uint16_t count = 2;
        getApplication()->scene().getComponents<Camera>(count, camera);
        camera[_camera2 ? 1 : 0]->activate();
    }

    ImGui::End();
}

void DepthPrePass::render(gerium_frame_graph_t frameGraph,
                          gerium_renderer_t renderer,
                          gerium_command_buffer_t commandBuffer,
                          gerium_uint32_t worker,
                          gerium_uint32_t totalWorkers) {
    auto& manager = getApplication()->resourceManager();
    auto& scene   = getApplication()->scene();
    auto camera   = scene.getActiveCamera();

    for (auto mesh : scene.visibleMeshes()) {
        if (mesh->getMaterial().getFlags() != DrawFlags::None &&
            mesh->getMaterial().getFlags() != DrawFlags::DoubleSided) {
            continue;
        }
        if (((gerium_buffer_h) mesh->getTangents()).unused == 65535) {
            continue;
        }
        gerium_command_buffer_bind_technique(commandBuffer, mesh->getMaterial().getTechnique());
        gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), 0);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, mesh->getMaterial().getDecriptorSet(), 1);
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh->getPositions(), 0, mesh->getPositionsOffset());
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, mesh->getTexcoords(), 1, mesh->getTexcoordsOffset());
        gerium_command_buffer_bind_index_buffer(
            commandBuffer, mesh->getIndices(), mesh->getIndicesOffset(), mesh->getIndexType());
        gerium_command_buffer_draw_indexed(commandBuffer, 0, mesh->getPrimitiveCount(), 0, 0, 1);
    }
}

void LightPass::render(gerium_frame_graph_t frameGraph,
                       gerium_renderer_t renderer,
                       gerium_command_buffer_t commandBuffer,
                       gerium_uint32_t worker,
                       gerium_uint32_t totalWorkers) {
    auto& manager  = getApplication()->resourceManager();
    auto& scene    = getApplication()->scene();
    auto camera    = scene.getActiveCamera();
    auto technique = manager.getTechnique("base");

    std::vector<Model*> models;
    models.resize(1000);
    gerium_uint16_t modelCount = 1000;
    scene.getComponents<Model>(modelCount, models.data());

    gerium_command_buffer_bind_technique(commandBuffer, technique);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, 0);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);

    ///////////////////
    // TODO: For test only

    gerium_uint32_t vertices = 0;

    auto dataV = (glm::vec3*) gerium_renderer_map_buffer(renderer, _vertices, 0, sizeof(glm::vec3) * _maxPoints);

    if (PresentPass::drawBBox()) {
        for (auto mesh : scene.visibleMeshes()) {
            if (mesh->getMaterial().getFlags() != DrawFlags::None &&
                mesh->getMaterial().getFlags() != DrawFlags::DoubleSided) {
                continue;
            }
            if (((gerium_buffer_h) mesh->getTangents()).unused == 65535) {
                continue;
            }
            const auto& bbox    = mesh->worldBoundingBox();
            const glm::vec3& p1 = bbox.min();
            const glm::vec3& p2 = bbox.max();

            dataV[vertices++] = glm::vec3(p1.x, p1.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p1.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p1.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p1.y, p2.z);
            dataV[vertices++] = glm::vec3(p2.x, p1.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p1.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p1.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p1.y, p1.z);

            dataV[vertices++] = glm::vec3(p1.x, p2.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p2.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p2.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p2.y, p2.z);
            dataV[vertices++] = glm::vec3(p2.x, p2.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p2.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p2.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p2.y, p1.z);

            dataV[vertices++] = glm::vec3(p1.x, p1.y, p1.z);
            dataV[vertices++] = glm::vec3(p1.x, p2.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p1.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p2.y, p1.z);
            dataV[vertices++] = glm::vec3(p2.x, p2.y, p2.z);
            dataV[vertices++] = glm::vec3(p2.x, p1.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p1.y, p2.z);
            dataV[vertices++] = glm::vec3(p1.x, p2.y, p2.z);

            if (vertices >= _maxPoints) {
                break;
            }
        }
    }

    if (PresentPass::camera2()) {
        auto primaryCamera = scene.getAnyComponentNode<Camera>();

        const auto nearPlane = 0.0f;
        const auto farPlane  = 0.995f;

        auto invV = glm::inverse(primaryCamera->viewProjection());

        const glm::vec4 point0 = invV * glm::vec4(-1.0f, -1.0f, nearPlane, 1.0f);
        const glm::vec4 point1 = invV * glm::vec4(-1.0f, -1.0f, farPlane, 1.0f);
        const glm::vec4 point2 = invV * glm::vec4(1.0f, -1.0f, nearPlane, 1.0f);
        const glm::vec4 point3 = invV * glm::vec4(1.0f, -1.0f, farPlane, 1.0f);
        const glm::vec4 point4 = invV * glm::vec4(-1.0f, 1.0f, nearPlane, 1.0f);
        const glm::vec4 point5 = invV * glm::vec4(-1.0f, 1.0f, farPlane, 1.0f);
        const glm::vec4 point6 = invV * glm::vec4(1.0f, 1.0f, nearPlane, 1.0f);
        const glm::vec4 point7 = invV * glm::vec4(1.0f, 1.0f, farPlane, 1.0f);

        dataV[vertices++] = glm::vec3(point0.xyz()) / point0.w;
        dataV[vertices++] = glm::vec3(point1.xyz()) / point1.w;
        dataV[vertices++] = glm::vec3(point2.xyz()) / point2.w;
        dataV[vertices++] = glm::vec3(point3.xyz()) / point3.w;
        dataV[vertices++] = glm::vec3(point4.xyz()) / point4.w;
        dataV[vertices++] = glm::vec3(point5.xyz()) / point5.w;
        dataV[vertices++] = glm::vec3(point6.xyz()) / point6.w;
        dataV[vertices++] = glm::vec3(point7.xyz()) / point7.w;

        dataV[vertices++] = glm::vec3(point0.xyz()) / point0.w;
        dataV[vertices++] = glm::vec3(point2.xyz()) / point2.w;
        dataV[vertices++] = glm::vec3(point2.xyz()) / point2.w;
        dataV[vertices++] = glm::vec3(point6.xyz()) / point6.w;
        dataV[vertices++] = glm::vec3(point6.xyz()) / point6.w;
        dataV[vertices++] = glm::vec3(point4.xyz()) / point4.w;
        dataV[vertices++] = glm::vec3(point4.xyz()) / point4.w;
        dataV[vertices++] = glm::vec3(point0.xyz()) / point0.w;

        dataV[vertices++] = glm::vec3(point1.xyz()) / point1.w;
        dataV[vertices++] = glm::vec3(point3.xyz()) / point3.w;
        dataV[vertices++] = glm::vec3(point3.xyz()) / point3.w;
        dataV[vertices++] = glm::vec3(point7.xyz()) / point7.w;
        dataV[vertices++] = glm::vec3(point7.xyz()) / point7.w;
        dataV[vertices++] = glm::vec3(point5.xyz()) / point5.w;
        dataV[vertices++] = glm::vec3(point5.xyz()) / point5.w;
        dataV[vertices++] = glm::vec3(point1.xyz()) / point1.w;
    }

    gerium_renderer_unmap_buffer(renderer, _vertices);

    if (vertices) {
        gerium_command_buffer_bind_technique(commandBuffer, _lines);
        gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), 0);
        gerium_command_buffer_bind_vertex_buffer(commandBuffer, _vertices, 0, 0);
        gerium_command_buffer_draw(commandBuffer, 0, vertices, 0, 1);
    }
}

void PresentPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    std::filesystem::path appDir = gerium_file_get_app_dir();

    _technique     = getApplication()->resourceManager().loadTechnique(appDir / "techniques" / "present.yaml");
    _descriptorSet = getApplication()->resourceManager().createDescriptorSet();

    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "light");
}

void PresentPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
    _technique     = nullptr;
}

bool PresentPass::_drawBBox{};

bool PresentPass::_camera2{};

void LightPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    std::filesystem::path appDir = gerium_file_get_app_dir();

    _descriptorSet = getApplication()->resourceManager().createDescriptorSet();
    gerium_renderer_bind_resource(renderer, _descriptorSet, 0, "color");
    gerium_renderer_bind_resource(renderer, _descriptorSet, 1, "normal");
    gerium_renderer_bind_resource(renderer, _descriptorSet, 2, "metallic_roughness");

    _maxPoints = 24 * 1000;
    _vertices  = getApplication()->resourceManager().createBuffer(
        GERIUM_BUFFER_USAGE_VERTEX_BIT, true, "", "lines_vertices", nullptr, sizeof(glm::vec3) * _maxPoints);
    _lines = getApplication()->resourceManager().loadTechnique(appDir / "techniques" / "lines.yaml");
}

void LightPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
    _lines         = nullptr;
    _vertices      = nullptr;
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
    _renderPasses.push_back(&renderPass);

    gerium_render_pass_t pass{ prepare, resize, render };
    gerium_frame_graph_add_pass(_frameGraph, renderPass.name().c_str(), &pass, &renderPass);
}

void Application::createScene() {
    std::filesystem::path appDir = gerium_file_get_app_dir();

    auto sponzaDir       = (appDir / "assets" / "models" / "sponza" / "Sponza.gltf").string();
    auto flightHelmetDir = (appDir / "assets" / "models" / "flight-helmet" / "FlightHelmet.gltf").string();

    auto modelSponza       = Model::loadGlTF(_renderer, _resourceManager, sponzaDir.c_str());
    auto modelFlightHelmet = Model::loadGlTF(_renderer, _resourceManager, flightHelmetDir.c_str());

    auto defaultTransform = Transform{ glm::identity<glm::mat4>() };
    auto sponzaTransform  = Transform{ glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.15f, 0.15f, 0.15f)) };

    auto root   = _scene.root();
    auto sponza = _scene.addNode(root);

    _scene.addComponentToNode(root, defaultTransform);
    _scene.addComponentToNode(root, Camera(_application, _resourceManager));
    auto camera2 = _scene.addComponentToNode(root, Camera(_application, _resourceManager));
    _scene.addComponentToNode(sponza, sponzaTransform);
    _scene.addComponentToNode(sponza, modelSponza);

    for (int x = -10; x < 10; ++x) {
        for (int y = -2; y < 2; ++y) {
            auto transform =
                Transform{ glm::rotate(glm::scale(glm::identity<glm::mat4>(), glm::vec3(0.125f, 0.125f, 0.125f)),
                                       glm::radians(-90.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f)) };
            transform.localMatrix =
                glm::translate(glm::identity<glm::mat4>(), glm::vec3(x * 0.1f, 0, y * 0.1f)) * transform.localMatrix;

            auto flightHelmet = _scene.addNode(root);
            _scene.addComponentToNode(flightHelmet, transform);
            _scene.addComponentToNode(flightHelmet, modelFlightHelmet);
        }
    }

    camera2->setPosition({ 0.0f, 3.2, -0.0f });
    camera2->setRotation(M_PI_2, -M_PI_2);
    camera2->activate();
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
    addPass(_gbufferPass);
    addPass(_depthPrePass);
    addPass(_lightPass);

    std::filesystem::path appDir = gerium_file_get_app_dir();
    _resourceManager.loadFrameGraph(appDir / "frame-graphs" / "main.yaml");
    _baseTechnique = _resourceManager.loadTechnique(appDir / "techniques" / "base.yaml");

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }

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
    _scene.culling();

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
