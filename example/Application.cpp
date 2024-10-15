#include "Application.hpp"

void CullingPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet0, SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet1, MESH_DATA_SET);
    gerium_command_buffer_dispatch(commandBuffer, 1, 1, 1);
}

void CullingPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    auto& datas     = application()->clusterDatas();
    _descriptorSet0 = application()->resourceManager().createDescriptorSet("", true);
    _descriptorSet1 = application()->resourceManager().createDescriptorSet("", true);
    gerium_renderer_bind_buffer(renderer, _descriptorSet0, 0, application()->drawData());
    gerium_renderer_bind_buffer(renderer, _descriptorSet1, 0, application()->instances());
    gerium_renderer_bind_buffer(renderer, _descriptorSet1, 1, datas.meshesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet1, 3, datas.meshletsBuffer);
}

void CullingPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet1 = nullptr;
    _descriptorSet0 = nullptr;
}

void GBufferPass::render(gerium_frame_graph_t frameGraph,
                         gerium_renderer_t renderer,
                         gerium_command_buffer_t commandBuffer,
                         gerium_uint32_t worker,
                         gerium_uint32_t totalWorkers) {
    auto camera = application()->getCamera();
    gerium_command_buffer_bind_technique(commandBuffer, application()->getBaseTechnique());
    gerium_command_buffer_bind_descriptor_set(commandBuffer, camera->getDecriptorSet(), SCENE_DATA_SET);
    gerium_command_buffer_bind_descriptor_set(commandBuffer, _descriptorSet, MESH_DATA_SET);
    gerium_command_buffer_draw_mesh_tasks_indirect(commandBuffer, application()->indirectDrawBuffer(), 0, 1, 4 * 3);
}

void GBufferPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    auto& datas    = application()->clusterDatas();
    _descriptorSet = application()->resourceManager().createDescriptorSet("", true);

    gerium_renderer_bind_buffer(renderer, _descriptorSet, 0, application()->instances());
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 1, datas.meshesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 2, datas.verticesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 3, datas.meshletsBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 4, datas.vertexIndicesBuffer);
    gerium_renderer_bind_buffer(renderer, _descriptorSet, 5, datas.primitiveIndicesBuffer);
}

void GBufferPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    _descriptorSet = nullptr;
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
        ImGui::Text("Meshlets: %s", application()->meshShaderSupported() ? "hardware" : "software");
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

void Application::createScene() {
    auto bunny  = loadClusterMesh(_clusterDatas, "bunny.obj");
    auto dragon = loadClusterMesh(_clusterDatas, "dragon.obj");
    uploadClusterDatas(_clusterDatas, 0);

    for (int i = 0; i < 25; ++i) {
        float x = (i % 5) * 2.0f;
        float z = (i / 5) * 2.0f;

        if (i % 2 == 0) {
            bunny.world        = glm::translate(glm::vec3(x, 0.0f, z));
            bunny.inverseWorld = glm::inverse(bunny.world);
            _instances.push_back(bunny);
        } else {
            dragon.world = glm::translate(glm::vec3(x, 0.0f, z)) * glm::scale(glm::vec3(1.0f, 1.0f, 1.0f) * 3.0f);
            dragon.inverseWorld = glm::inverse(dragon.world);
            _instances.push_back(dragon);
        }
    }

    while (_instances.size() % TASK_GROUP_SIZE != 0) {
        ClusterMeshInstance padding{};
        padding.mesh = 0xFFFFFFFF;
        _instances.push_back(padding);
    }

    DrawData drawData{};
    drawData.drawCount = glm::uint(_instances.size());
    drawData.lodTarget = 0;
    _drawData =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT, false, "draw_data", &drawData, sizeof(drawData));

    _instancesBuffer = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                     false,
                                                     "instances",
                                                     _instances.data(),
                                                     sizeof(_instances[0]) * _instances.size());

    struct IndirectCommand {
        uint32_t groupCountX;
        uint32_t groupCountY;
        uint32_t groupCountZ;
    };

    IndirectCommand command{};
    command.groupCountX = uint32_t(_instances.size());
    command.groupCountY = 1;
    command.groupCountZ = 1;

    _indirectDrawBuffer = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_INDIRECT_BIT, false, "indirect_draw", &command, sizeof(command));
}

void Application::uploadClusterDatas(ClusterDatas& clusterDatas, gerium_uint32_t id) {
    auto strId = '_' + std::to_string(id);
    clusterDatas.verticesBuffer =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                      false,
                                      "vertices_buffer" + strId,
                                      clusterDatas.vertices.data(),
                                      sizeof(clusterDatas.vertices[0]) * clusterDatas.vertices.size());

    clusterDatas.meshletsBuffer =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                      false,
                                      "meshlets_buffer" + strId,
                                      clusterDatas.meshlets.data(),
                                      sizeof(clusterDatas.meshlets[0]) * clusterDatas.meshlets.size());

    clusterDatas.meshesBuffer =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                      false,
                                      "meshes_buffer" + strId,
                                      clusterDatas.meshes.data(),
                                      sizeof(clusterDatas.meshes[0]) * clusterDatas.meshes.size());

    clusterDatas.vertexIndicesBuffer =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                      false,
                                      "vertex_indices_buffer" + strId,
                                      clusterDatas.vertexIndices.data(),
                                      sizeof(clusterDatas.vertexIndices[0]) * clusterDatas.vertexIndices.size());

    clusterDatas.primitiveIndicesBuffer =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                      false,
                                      "primitive_indices_buffer" + strId,
                                      clusterDatas.primitiveIndices.data(),
                                      sizeof(clusterDatas.primitiveIndices[0]) * clusterDatas.primitiveIndices.size());

    clusterDatas.vertices.clear();
    clusterDatas.meshlets.clear();
    clusterDatas.meshes.clear();
    clusterDatas.vertexIndices.clear();
    clusterDatas.primitiveIndices.clear();
}

ClusterMeshInstance Application::loadClusterMesh(ClusterDatas& clusterDatas, std::string_view name) const {
    ClusterMeshInstance result{};

    std::filesystem::path appDir = gerium_file_get_app_dir();
    auto fileName                = (appDir / "models" / name).string();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    auto resultLoad = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName.c_str());

    if (!warn.empty()) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_WARNING, warn.c_str());
    }

    if (!err.empty()) {
        gerium_logger_print(_logger, GERIUM_LOGGER_LEVEL_WARNING, err.c_str());
    }

    if (!resultLoad) {
        throw std::runtime_error("Load .obj model failed");
    }

    const auto& mesh = shapes[0].mesh;
    auto indexCount  = mesh.indices.size();
    std::vector<uint32_t> remap(indexCount);
    std::vector<uint32_t> indicesOrigin(indexCount);
    std::vector<VertexOptimized> verticesOrigin(attrib.vertices.size());

    for (size_t i = 0; i < indexCount; ++i) {
        auto vi = mesh.indices[i].vertex_index * 3;
        auto ni = mesh.indices[i].normal_index * 3;
        // auto ti = mesh.indices[i].texcoord_index * 2;

        indicesOrigin[i] = vi / 3;

        auto pos = glm::vec3(attrib.vertices[vi + 0], attrib.vertices[vi + 1], attrib.vertices[vi + 2]);
        auto n   = glm::vec3(attrib.normals[ni + 0], attrib.normals[ni + 1], attrib.normals[ni + 2]) * 127.0f + 127.5f;
        // auto uv  = glm::vec2(attrib.texcoords[ti + 0], attrib.texcoords[ti + 1]);

        verticesOrigin[indicesOrigin[i]].position = glm::vec4(pos, 1.0f);
        verticesOrigin[indicesOrigin[i]].normal   = glm::u8vec4(uint8_t(n.x), uint8_t(n.y), uint8_t(n.z), uint8_t(0));
        // verticesOrigin[indicesOrigin[i]].texcoord.x = meshopt_quantizeHalf(uv.x);
        // verticesOrigin[indicesOrigin[i]].texcoord.y = meshopt_quantizeHalf(uv.y);
    }

    auto vertexCount = meshopt_generateVertexRemap(remap.data(),
                                                   indicesOrigin.data(),
                                                   indexCount,
                                                   verticesOrigin.data(),
                                                   verticesOrigin.size(),
                                                   sizeof(VertexOptimized));

    auto offsetVertices = clusterDatas.vertices.size();
    clusterDatas.vertices.resize(offsetVertices + vertexCount);
    VertexOptimized* vertices = clusterDatas.vertices.data() + offsetVertices;
    std::vector<uint32_t> indices(indexCount);

    meshopt_remapVertexBuffer(
        vertices, verticesOrigin.data(), verticesOrigin.size(), sizeof(VertexOptimized), remap.data());
    meshopt_remapIndexBuffer(indices.data(), indicesOrigin.data(), indexCount, remap.data());

    meshopt_optimizeVertexCache(indices.data(), indicesOrigin.data(), indexCount, vertexCount);
    meshopt_optimizeVertexFetch(vertices, indices.data(), indexCount, vertices, vertexCount, sizeof(VertexOptimized));

    result.mesh = glm::uint(clusterDatas.meshes.size());
    clusterDatas.meshes.push_back({});
    auto& meshLods = clusterDatas.meshes.back();

    while (meshLods.lodCount < std::size(meshLods.lods)) {
        auto& lod = meshLods.lods[meshLods.lodCount++];

        lod.meshletOffset = uint32_t(clusterDatas.meshlets.size());
        lod.meshletCount  = uint32_t(appendMeshlets(clusterDatas, vertices, offsetVertices, vertexCount, indices));

        if (meshLods.lodCount < std::size(meshLods.lods)) {
            size_t nextIndicesTarget = size_t(double(indices.size()) * 0.75);
            size_t nextIndices       = meshopt_simplify(indices.data(),
                                                  indices.data(),
                                                  indices.size(),
                                                  &vertices->position.x,
                                                  vertexCount,
                                                  sizeof(VertexOptimized),
                                                  nextIndicesTarget,
                                                  1e-2f);
            assert(nextIndices <= indices.size());

            if (nextIndices == indices.size()) {
                break;
            }

            indices.resize(nextIndices);
            meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertexCount);
        }
    }

    return result;
}

size_t Application::appendMeshlets(ClusterDatas& clusterDatas,
                                   const VertexOptimized* vertices,
                                   size_t verticesOffset,
                                   size_t verticesCount,
                                   const std::vector<uint32_t>& indices) const {
    constexpr size_t maxVertices  = MESH_MAX_VERTICES;
    constexpr size_t maxTriangles = MESH_MAX_PRIMITIVES;
    constexpr float coneWeight    = 0.5f;

    std::vector<meshopt_Meshlet> meshlets(meshopt_buildMeshletsBound(indices.size(), maxVertices, maxTriangles));
    std::vector<unsigned int> meshletVertices(meshlets.size() * maxVertices);
    std::vector<unsigned char> meshletTriangles(meshlets.size() * maxTriangles * 3);

    meshlets.resize(meshopt_buildMeshlets(meshlets.data(),
                                          meshletVertices.data(),
                                          meshletTriangles.data(),
                                          indices.data(),
                                          indices.size(),
                                          &vertices[0].position.x,
                                          verticesCount,
                                          sizeof(VertexOptimized),
                                          maxVertices,
                                          maxTriangles,
                                          coneWeight));

    for (auto& meshlet : meshlets) {
        meshopt_optimizeMeshlet(&meshletVertices[meshlet.vertex_offset],
                                &meshletTriangles[meshlet.triangle_offset],
                                meshlet.triangle_count,
                                meshlet.vertex_count);

        const auto vertexOffset    = clusterDatas.vertexIndices.size();
        const auto primitiveOffset = clusterDatas.primitiveIndices.size();

        for (unsigned int i = 0; i < meshlet.vertex_count; ++i) {
            clusterDatas.vertexIndices.push_back(verticesOffset + meshletVertices[meshlet.vertex_offset + i]);
        }

        for (unsigned int i = 0; i < meshlet.triangle_count * 3; ++i) {
            clusterDatas.primitiveIndices.push_back(meshletTriangles[meshlet.triangle_offset + i]);
        }

        const auto bounds = meshopt_computeMeshletBounds(&meshletVertices[meshlet.vertex_offset],
                                                         &meshletTriangles[meshlet.triangle_offset],
                                                         meshlet.triangle_count,
                                                         &vertices[0].position.x,
                                                         verticesCount,
                                                         sizeof(VertexOptimized));

        MeshletOptimized meshletInfo = {};
        meshletInfo.vertexOffset     = uint32_t(vertexOffset);
        meshletInfo.primitiveOffset  = uint32_t(primitiveOffset);
        meshletInfo.vertexCount      = meshlet.vertex_count;
        meshletInfo.primitiveCount   = meshlet.triangle_count;

        meshletInfo.centerAndRadius = glm::vec4(bounds.center[0], bounds.center[1], bounds.center[2], bounds.radius);
        meshletInfo.coneAxisAndCutoff =
            glm::i8vec4(bounds.cone_axis_s8[0], bounds.cone_axis_s8[1], bounds.cone_axis_s8[2], bounds.cone_cutoff_s8);

        clusterDatas.meshlets.push_back(meshletInfo);
    }

    return meshlets.size();
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
    addPass(_cullingPass);

    std::filesystem::path appDir = gerium_file_get_app_dir();
    _resourceManager.loadFrameGraph((appDir / "frame-graphs" / "main.yaml").string());
    _baseTechnique = _resourceManager.loadTechnique((appDir / "techniques" / "base.yaml").string());

    createScene();
    _camera = Camera(_application, _resourceManager);

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }
}

void Application::uninitialize() {
    if (_renderer) {
        _indirectDrawBuffer = {};
        _instancesBuffer    = {};
        _clusterDatas       = {};

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

    _resourceManager.update(elapsedMs);

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
