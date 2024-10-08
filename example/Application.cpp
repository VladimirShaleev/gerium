#include "Application.hpp"

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto camera = application()->getCamera();
    auto& model = application()->model();
    auto& lod0  = model.meshes[0].lods[0];

    auto ds = application()->resourceManager().createDescriptorSet("");
    gerium_renderer_bind_buffer(renderer, ds, 0, model.verticesBuffer);
    gerium_renderer_bind_buffer(renderer, ds, 1, model.meshletDataBuffer);
    gerium_renderer_bind_buffer(renderer, ds, 2, model.meshletBuffer);

    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, MESH_DATA_SET);
    gerium_command_buffer_draw_mesh_task(commandBuffer, lod0.meshletCount, 1, 1);
}

void PresentPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    static bool drawProfiler = false;

    auto ds = application()->resourceManager().createDescriptorSet("");
    gerium_renderer_bind_resource(renderer, ds, 0, "color");

    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, ds, 0);
    gerium_command_buffer_draw(commandBuffer, 0, 3, 0, 1);

    if (drawProfiler) {
        gerium_bool_t show = drawProfiler;
        gerium_command_buffer_draw_profiler(commandBuffer, &show);
        drawProfiler = show;
    }

    if (ImGui::Begin("Settings")) {
        ImGui::LabelText(application()->meshShaderSupported() ? "hardware" : "software", "Meshlets");
        ImGui::Separator();
        ImGui::Checkbox("Show profiler", &drawProfiler);
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

size_t appendMeshlets(Model& result, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    const size_t max_vertices  = MESH_MAX_VERTICES;
    const size_t max_triangles = MESH_MAX_PRIMITIVES;
    const float cone_weight    = 0.5f;

    std::vector<meshopt_Meshlet> meshlets(meshopt_buildMeshletsBound(indices.size(), max_vertices, max_triangles));
    std::vector<unsigned int> meshlet_vertices(meshlets.size() * max_vertices);
    std::vector<unsigned char> meshlet_triangles(meshlets.size() * max_triangles * 3);

    meshlets.resize(meshopt_buildMeshlets(meshlets.data(),
                                          meshlet_vertices.data(),
                                          meshlet_triangles.data(),
                                          indices.data(),
                                          indices.size(),
                                          &vertices[0].position.x,
                                          vertices.size(),
                                          sizeof(Vertex),
                                          max_vertices,
                                          max_triangles,
                                          cone_weight));

    // note: we can append meshlet_vertices & meshlet_triangles buffers more or less directly with small changes in
    // Meshlet struct, but for now keep the GPU side layout flexible and separate
    for (auto& meshlet : meshlets) {
        meshopt_optimizeMeshlet(&meshlet_vertices[meshlet.vertex_offset],
                                &meshlet_triangles[meshlet.triangle_offset],
                                meshlet.triangle_count,
                                meshlet.vertex_count);

        size_t dataOffset = result.meshletdata.size();

        for (unsigned int i = 0; i < meshlet.vertex_count; ++i) {
            result.meshletdata.push_back(meshlet_vertices[meshlet.vertex_offset + i]);
        }

        const unsigned int* indexGroups =
            reinterpret_cast<const unsigned int*>(&meshlet_triangles[0] + meshlet.triangle_offset);
        unsigned int indexGroupCount = (meshlet.triangle_count * 3 + 3) / 4;

        for (unsigned int i = 0; i < indexGroupCount; ++i) {
            result.meshletdata.push_back(indexGroups[i]);
        }

        meshopt_Bounds bounds = meshopt_computeMeshletBounds(&meshlet_vertices[meshlet.vertex_offset],
                                                             &meshlet_triangles[meshlet.triangle_offset],
                                                             meshlet.triangle_count,
                                                             &vertices[0].position.x,
                                                             vertices.size(),
                                                             sizeof(Vertex));

        Meshlet m       = {};
        m.dataOffset    = uint32_t(dataOffset);
        m.triangleCount = meshlet.triangle_count;
        m.vertexCount   = meshlet.vertex_count;

        m.center       = glm::vec3(bounds.center[0], bounds.center[1], bounds.center[2]);
        m.radius       = bounds.radius;
        m.cone_axis[0] = bounds.cone_axis_s8[0];
        m.cone_axis[1] = bounds.cone_axis_s8[1];
        m.cone_axis[2] = bounds.cone_axis_s8[2];
        m.cone_cutoff  = bounds.cone_cutoff_s8;

        result.meshlets.push_back(m);
    }

    return meshlets.size();
}

void Application::createScene() {
    std::filesystem::path appDir = gerium_file_get_app_dir();
    auto fileName                = (appDir / "models" / "bunny.obj").string();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    auto result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName.c_str());

    if (!warn.empty()) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_WARNING, warn.c_str());
    }

    if (!err.empty()) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_WARNING, err.c_str());
    }

    if (!result) {
        throw std::runtime_error("Load .obj model failed");
    }

    auto indexCount = shapes[0].mesh.indices.size();
    std::vector<uint32_t> remap(indexCount);
    std::vector<uint32_t> indicesOrigin(indexCount);
    std::vector<Vertex> verticesOrigin(attrib.vertices.size());

    for (size_t i = 0; i < indexCount; ++i) {
        auto vi = shapes[0].mesh.indices[i].vertex_index;
        auto ni = shapes[0].mesh.indices[i].normal_index;
        auto ti = shapes[0].mesh.indices[i].texcoord_index;

        indicesOrigin[i] = vi;

        auto pos = glm::vec3(attrib.vertices[vi + 0], attrib.vertices[vi + 1], attrib.vertices[vi + 2]);
        auto n   = glm::vec3(attrib.normals[ni + 0], attrib.normals[ni + 1], attrib.normals[ni + 2]) * 127.0f + 127.5f;
        auto uv  = glm::vec2(attrib.texcoords[ti + 0], attrib.texcoords[ti + 1]);

        verticesOrigin[indicesOrigin[i]].position   = glm::vec4(pos, 1.0f);
        verticesOrigin[indicesOrigin[i]].normal     = glm::u8vec4(uint8_t(n.x), uint8_t(n.y), uint8_t(n.z), uint8_t(0));
        verticesOrigin[indicesOrigin[i]].texcoord.x = meshopt_quantizeHalf(uv.x);
        verticesOrigin[indicesOrigin[i]].texcoord.y = meshopt_quantizeHalf(uv.y);
    }

    auto vertexCount = meshopt_generateVertexRemap(
        remap.data(), indicesOrigin.data(), indexCount, verticesOrigin.data(), verticesOrigin.size(), sizeof(Vertex));

    std::vector<Vertex> vertices(vertexCount);
    std::vector<uint32_t> indices(indexCount);

    meshopt_remapVertexBuffer(
        vertices.data(), verticesOrigin.data(), verticesOrigin.size(), sizeof(Vertex), remap.data());
    meshopt_remapIndexBuffer(indices.data(), indicesOrigin.data(), indexCount, remap.data());

    meshopt_optimizeVertexCache(indices.data(), indicesOrigin.data(), indexCount, vertexCount);
    meshopt_optimizeVertexFetch(
        vertices.data(), indices.data(), indexCount, vertices.data(), vertexCount, sizeof(Vertex));

    glm::vec3 center = glm::vec3(0.0f);
    for (const auto& v : vertices) {
        center += v.position.xyz();
    }
    center /= float(vertices.size());

    float radius = 0;
    for (const auto& v : vertices) {
        radius = std::max(radius, glm::distance(center, v.position.xyz()));
    }

    Mesh mesh{};
    mesh.center       = center;
    mesh.radius       = radius;
    mesh.vertexOffset = 0;
    mesh.vertexCount  = gerium_uint32_t(vertices.size());

    _model.vertices.insert(_model.vertices.end(), vertices.begin(), vertices.end());

    std::vector<uint32_t> lodIndices = indices;
    while (mesh.lodCount < std::size(mesh.lods)) {
        auto& lod = mesh.lods[mesh.lodCount++];

        lod.indexOffset = uint32_t(_model.indices.size());
        lod.indexCount  = uint32_t(lodIndices.size());

        _model.indices.insert(_model.indices.end(), lodIndices.begin(), lodIndices.end());

        lod.meshletOffset = uint32_t(_model.meshlets.size());
        lod.meshletCount  = uint32_t(appendMeshlets(_model, vertices, lodIndices));

        if (mesh.lodCount < std::size(mesh.lods)) {
            size_t nextIndicesTarget = size_t(double(lodIndices.size()) * 0.75);
            size_t nextIndices       = meshopt_simplify(lodIndices.data(),
                                                  lodIndices.data(),
                                                  lodIndices.size(),
                                                  &vertices[0].position.x,
                                                  vertices.size(),
                                                  sizeof(Vertex),
                                                  nextIndicesTarget,
                                                  1e-2f);
            assert(nextIndices <= lodIndices.size());

            if (nextIndices == lodIndices.size()) {
                break;
            }

            lodIndices.resize(nextIndices);
            meshopt_optimizeVertexCache(lodIndices.data(), lodIndices.data(), lodIndices.size(), vertexCount);
        }
    }

    while (_model.meshlets.size() % 64) {
        _model.meshlets.push_back(Meshlet());
    }

    _model.meshes.push_back(mesh);

    auto& lod0 = _model.meshes[0].lods[0];

    _model.meshletBuffer = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                         false,
                                                         "meshlets",
                                                         &_model.meshlets[lod0.meshletOffset],
                                                         sizeof(Meshlet) * lod0.meshletCount);

    _model.meshletDataBuffer = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                             false,
                                                             "meshlet_data",
                                                             _model.meshletdata.data(),
                                                             sizeof(_model.meshletdata[0]) * _model.meshletdata.size());
    _model.verticesBuffer    = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                          false,
                                                          "vertices_buffer",
                                                          vertices.data(),
                                                          sizeof(_model.vertices[0]) * _model.vertices.size());
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
        _model = {};

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
