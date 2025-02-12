#include "RenderService.hpp"
#include "../Application.hpp"
#include "../Model.hpp"
#include "../components/Camera.hpp"
#include "../components/MeshIndices.hpp"
#include "../components/WorldTransform.hpp"
#include "../passes/CullingPass.hpp"
#include "../passes/GBufferPass.hpp"
#include "../passes/PresentPass.hpp"
#include "SceneService.hpp"

#include <ranges>

gerium_technique_h RenderService::baseTechnique() const noexcept {
    return _baseTechnique;
}

ResourceManager& RenderService::resourceManager() noexcept {
    return _resourceManager;
}

// gerium_descriptor_set_h RenderService::getSceneDescriptorSet(Entity entity) {
//     auto& camera = entityRegistry().getComponent<Camera>(entity);
//
//     if (auto it = _cameras.find(entity); it != _cameras.end()) {
//         return it->second.first;
//     }
//
//     const auto name = entityRegistry().getComponent<Name>(entity).name;
//
//     auto ds     = _resourceManager.createDescriptorSet(name + "_ds");
//     auto buffer = _resourceManager.createBuffer(
//         GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, name + "_data", nullptr, sizeof(SceneData));
//
//     gerium_renderer_bind_buffer(_renderer, ds, 0, buffer);
//
//     auto data = (SceneData*) gerium_renderer_map_buffer(_renderer, buffer, 0, 0);
//
//     data->view               = camera.view;
//     data->viewProjection     = camera.viewProjection;
//     data->prevViewProjection = camera.prevViewProjection;
//     data->invViewProjection  = glm::inverse((glm::mat4&) camera.viewProjection);
//     data->viewPosition       = glm::vec4((glm::vec3&) camera.position, 1.0f);
//
//     gerium_renderer_unmap_buffer(_renderer, buffer);
//
//     _cameras[entity] = { ds, buffer };
//     return ds;
// }
//
// gerium_descriptor_set_h RenderService::getActiveSceneDescriptorSet() {
//     for (auto& camera : entityRegistry().getAllComponents<Camera>()) {
//         if (camera.active) {
//             return getSceneDescriptorSet(entityRegistry().getEntityFromComponent(camera));
//         }
//     }
//     return { UndefinedHandle };
// }

gerium_descriptor_set_h RenderService::clusterDescriptorSet() const noexcept {
    return _cluster.ds;
}

gerium_descriptor_set_h RenderService::sceneDataDescriptorSet() const noexcept {
    return _activeCameraDs;
}

gerium_buffer_h RenderService::instancesBuffer() const noexcept {
    return _staticInstances;
}

gerium_buffer_h RenderService::instanceCountBuffer() const noexcept {
    return _staticInstanceCountBuff;
}

gerium_uint32_t RenderService::instanceCount() const noexcept {
    return _staticInstanceCount;
}

void RenderService::loadModel(const std::string& filename) {
}

void RenderService::createCluster(const Cluster& cluster) {
    if (!_8and16BitStorageSupported) {
        throw std::runtime_error("Support for 8-bit and 16-bit storage is currently mandatory");
    }

    _cluster = {};

    {
        std::vector<Vertex> vertices(cluster.vertices.size());
        for (size_t i = 0; i < cluster.vertices.size(); ++i) {
            const auto& v = cluster.vertices[i];

            auto n = glm::vec3(v.nx, v.ny, v.nz) * 127.0f + 127.5f;
            auto t = glm::vec3(v.tx, v.ty, v.tz) * 127.0f + 127.5f;
            auto s = v.ts < 0.0f ? -1 : 1;

            vertices[i].px = meshopt_quantizeHalf(v.px);
            vertices[i].py = meshopt_quantizeHalf(v.py);
            vertices[i].pz = meshopt_quantizeHalf(v.pz);
            vertices[i].nx = uint8_t(n.x);
            vertices[i].ny = uint8_t(n.y);
            vertices[i].nz = uint8_t(n.z);
            vertices[i].tx = uint8_t(t.x);
            vertices[i].ty = uint8_t(t.y);
            vertices[i].tz = uint8_t(t.z);
            vertices[i].ts = int8_t(s);
            vertices[i].tu = meshopt_quantizeHalf(v.tu);
            vertices[i].tv = meshopt_quantizeHalf(v.tv);
        }

        _cluster.vertices =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                          false,
                                          "cluster_vertices",
                                          (gerium_cdata_t) vertices.data(),
                                          (gerium_uint32_t) (vertices.size() * sizeof(Vertex)));
    }

    {
        _cluster.indices =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                          false,
                                          "cluster_indices",
                                          (gerium_cdata_t) cluster.primitiveIndices.data(),
                                          (gerium_uint32_t) (cluster.primitiveIndices.size() * sizeof(uint32_t)));
    }

    {
        std::vector<Mesh> meshes(cluster.meshes.size());
        for (size_t i = 0; i < cluster.meshes.size(); ++i) {
            const auto& m = cluster.meshes[i];

            meshes[i].center[0]    = meshopt_quantizeHalf(m.center[0]);
            meshes[i].center[1]    = meshopt_quantizeHalf(m.center[1]);
            meshes[i].center[2]    = meshopt_quantizeHalf(m.center[2]);
            meshes[i].radius       = meshopt_quantizeHalf(m.radius);
            meshes[i].bboxMin[0]   = meshopt_quantizeHalf(m.bboxMin[0]);
            meshes[i].bboxMin[1]   = meshopt_quantizeHalf(m.bboxMin[1]);
            meshes[i].bboxMin[2]   = meshopt_quantizeHalf(m.bboxMin[2]);
            meshes[i].bboxMax[0]   = meshopt_quantizeHalf(m.bboxMax[0]);
            meshes[i].bboxMax[1]   = meshopt_quantizeHalf(m.bboxMax[1]);
            meshes[i].bboxMax[2]   = meshopt_quantizeHalf(m.bboxMax[2]);
            meshes[i].vertexOffset = m.vertexOffset;
            meshes[i].vertexCount  = m.vertexCount;
            meshes[i].lodCount     = uint8_t(m.lodCount);
            for (int l = 0; l < std::size(m.lods); ++l) {
                meshes[i].lods[l] = m.lods[l];
            }
        }

        _cluster.meshes =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                          false,
                                          "cluster_meshes",
                                          (gerium_cdata_t) meshes.data(),
                                          (gerium_uint32_t) (meshes.size() * sizeof(Mesh)));
    }

    _cluster.ds = _resourceManager.createDescriptorSet("cluster_ds", true);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 0, _cluster.vertices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 1, _cluster.indices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 2, _cluster.meshes);
}

void RenderService::createStaticInstances() {
    auto view = entityRegistry().view<MeshIndices, WorldTransform>();

    std::vector<MeshInstance> instances;

    for (auto entity : view) {
        auto& meshIndices    = view.get<MeshIndices>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);

        for (auto mesh : meshIndices.meshes) {
            instances.push_back({});
            auto& instance = instances.back();

            instance.world        = worldTransform.matrix;
            instance.prevWorld    = worldTransform.matrix;
            instance.normalMatrix = glm::transpose(glm::inverse(worldTransform.matrix));
            instance.scale        = worldTransform.scale;
            instance.mesh         = mesh;
        }
    }

    _staticInstances = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                     false,
                                                     "static_instances",
                                                     (gerium_cdata_t) instances.data(),
                                                     (gerium_uint32_t) (instances.size() * sizeof(instances[0])),
                                                     0);

    _staticInstanceCount = (gerium_uint32_t) instances.size();

    _staticInstanceCountBuff = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                             false,
                                                             "static_instance_count",
                                                             (gerium_cdata_t) &_staticInstanceCount,
                                                             sizeof(gerium_uint32_t),
                                                             0);
}

void RenderService::start() {
    gerium_renderer_options_t options{};
    options.app_version               = GERIUM_VERSION_ENCODE(1, 0, 0);
    options.command_buffers_per_frame = 5;
    options.descriptor_sets_pool_size = 128;
    options.descriptor_pool_elements  = 128;
    options.dynamic_ssbo_size         = 64 * 1024 * 1024;

#ifdef NDEBUG
    options.debug_mode = false;
#else
    options.debug_mode = true;
#endif

    constexpr auto features = GERIUM_FEATURE_DRAW_INDIRECT_BIT | GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT |
                              GERIUM_FEATURE_8_BIT_STORAGE_BIT | GERIUM_FEATURE_16_BIT_STORAGE_BIT |
                              GERIUM_FEATURE_BINDLESS_BIT;

    check(gerium_renderer_create(application().handle(), features, &options, &_renderer));
    gerium_renderer_set_profiler_enable(_renderer, true);
    check(gerium_profiler_create(_renderer, &_profiler));

    const auto supportedFeatures = gerium_renderer_get_enabled_features(_renderer);
    _bindlessSupported           = supportedFeatures & GERIUM_FEATURE_BINDLESS_BIT;
    _drawIndirectSupported       = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_BIT;
    _drawIndirectCountSupported  = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT;
    _8and16BitStorageSupported   = (supportedFeatures & GERIUM_FEATURE_8_BIT_STORAGE_BIT) &&
                                 (supportedFeatures & GERIUM_FEATURE_16_BIT_STORAGE_BIT);

    if (!_drawIndirectSupported || !_drawIndirectCountSupported) {
        throw std::runtime_error("Support draw indirect and draw indirect count is currently mandatory");
    }

    check(gerium_frame_graph_create(_renderer, &_frameGraph));

    _resourceManager.create(_renderer, _frameGraph);

    addPass<PresentPass>();
    addPass<GBufferPass>();
    addPass<CullingPass>();

    _resourceManager.loadFrameGraph(GRAPH_MAIN_ID);
    check(gerium_frame_graph_compile(_frameGraph));

    _baseTechnique = _resourceManager.loadTechnique(TECH_BASE_ID);
    _resourceManager.loadTechnique(TECH_OTHER_ID);

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }

    _activeCamera = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_VERTEX_BIT | GERIUM_BUFFER_USAGE_UNIFORM_BIT,
                                                  true,
                                                  "active_camera",
                                                  nullptr,
                                                  sizeof(SceneData));

    _activeCameraDs = _resourceManager.createDescriptorSet("scene_data", true);
    gerium_renderer_bind_buffer(_renderer, _activeCameraDs, 0, _activeCamera);
}

void RenderService::stop() {
    if (_renderer) {
        for (auto& _renderPasse : std::ranges::reverse_view(_renderPasses)) {
            _renderPasse->uninitialize(_frameGraph, _renderer);
        }

        _activeCameraDs          = {};
        _activeCamera            = {};
        _staticInstanceCountBuff = {};
        _staticInstances         = {};
        _cluster                 = {};

        _baseTechnique = nullptr;

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

void RenderService::update(gerium_uint64_t elapsedMs, gerium_float64_t /* elapsed */) {
    if (gerium_renderer_new_frame(_renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return;
    }

    _resourceManager.update(elapsedMs);

    auto view = entityRegistry().view<Camera>();

    Camera* camera = nullptr;
    for (auto entity : view) {
        auto& c = view.get<Camera>(entity);
        if (c.active) {
            camera = &c;
        }
    }
    assert(camera);

    auto projectionT = glm::transpose(camera->projection);

    vec4 frustumX = projectionT[3] + projectionT[0];
    vec4 frustumY = projectionT[3] + projectionT[1];
    frustumX /= glm::length(frustumX.xyz());
    frustumY /= glm::length(frustumY.xyz());

    auto invResolution = glm::vec2{ 1.0f / camera->resolution.x, 1.0f / camera->resolution.y };

    auto p00p11 = glm::vec2(camera->projection[0][0], camera->projection[1][1]);

    auto sceneData            = (SceneData*) gerium_renderer_map_buffer(_renderer, _activeCamera, 0, sizeof(SceneData));
    sceneData->view           = camera->view;
    sceneData->viewProjection = camera->viewProjection;
    sceneData->prevViewProjection = camera->prevViewProjection;
    sceneData->invViewProjection  = glm::inverse(camera->viewProjection);
    sceneData->viewPosition       = glm::vec4(camera->position, 1.0f);
    sceneData->eye                = glm::vec4(camera->front, 1.0f);
    sceneData->frustum            = vec4(frustumX.x, frustumX.z, frustumY.y, frustumY.z);
    sceneData->p00p11             = p00p11;
    sceneData->farNear            = glm::vec2(camera->farPlane, camera->nearPlane);
    sceneData->invResolution      = invResolution;
    sceneData->resolution         = camera->resolution;
    sceneData->lodTarget          = (2.0f / p00p11.y) * invResolution.x;
    gerium_renderer_unmap_buffer(_renderer, _activeCamera);

    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);

    if (_error) {
        std::rethrow_exception(_error);
    }
}

gerium_uint32_t RenderService::prepare(gerium_frame_graph_t frameGraph,
                                       gerium_renderer_t renderer,
                                       gerium_uint32_t maxWorkers,
                                       gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        return renderPass->prepare(frameGraph, renderer, maxWorkers);
    } catch (...) {
        renderPass->renderService()._error = std::current_exception();
        return 0;
    }
}

gerium_bool_t RenderService::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        renderPass->resize(frameGraph, renderer);
    } catch (...) {
        renderPass->renderService()._error = std::current_exception();
        return false;
    }
    return true;
}

gerium_bool_t RenderService::render(gerium_frame_graph_t frameGraph,
                                    gerium_renderer_t renderer,
                                    gerium_command_buffer_t commandBuffer,
                                    gerium_uint32_t worker,
                                    gerium_uint32_t totalWorkers,
                                    gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        renderPass->render(frameGraph, renderer, commandBuffer, worker, totalWorkers);
    } catch (...) {
        renderPass->renderService()._error = std::current_exception();
        return false;
    }
    return true;
}
