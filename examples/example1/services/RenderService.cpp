#include "RenderService.hpp"
#include "../Application.hpp"
#include "../Model.hpp"
#include "../components/Camera.hpp"
#include "../components/Static.hpp"
#include "../passes/CullingPass.hpp"
#include "../passes/GBufferPass.hpp"
#include "../passes/MergeInstancesPass.hpp"
#include "../passes/PresentPass.hpp"
#include "SceneService.hpp"

#include <ranges>

using namespace entt::literals;

void RenderService::mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer) {
    if (_dynamicMaterialsCount) {
        const auto size = _8and16BitStorageSupported ? sizeof(Material) : sizeof(MaterialNonCompressed);
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _dynamicMaterials,
                                          0,
                                          _materials[_frameIndex],
                                          _staticMaterialsCount * size,
                                          _dynamicMaterialsCount * size);
    }
    if (_dynamicInstancesCount) {
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _dynamicInstances,
                                          0,
                                          _instances[_frameIndex],
                                          _staticInstancesCount * sizeof(MeshInstance),
                                          _dynamicInstancesCount * sizeof(MeshInstance));
    }
}

void RenderService::start() {
    gerium_renderer_options_t options{};
    options.app_version               = GERIUM_VERSION_ENCODE(1, 0, 0);
    options.command_buffers_per_frame = 5;
    options.descriptor_sets_pool_size = 128;
    options.descriptor_pool_elements  = 128;
    options.dynamic_ssbo_size         = 64 * 1024 * 1024;
#ifndef NDEBUG
    options.debug_mode = true; // includes validation layers, GPU object names and logs
#endif

    // Requested features needed for this example
    constexpr auto features = GERIUM_FEATURE_DRAW_INDIRECT_BIT | GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT |
                              GERIUM_FEATURE_8_BIT_STORAGE_BIT | GERIUM_FEATURE_16_BIT_STORAGE_BIT |
                              GERIUM_FEATURE_BINDLESS_BIT;

    // Initializes the gerium rendering system
    check(gerium_renderer_create(application().handle(), features, &options, &_renderer));

    // Enables queries to GPU to obtain performance timestamps
    // and and creates an object to receive these metrics for the client code
    gerium_renderer_set_profiler_enable(_renderer, true);
    check(gerium_profiler_create(_renderer, &_profiler));

    // Checking supported features requested when creating a render system
    const auto supportedFeatures = gerium_renderer_get_enabled_features(_renderer);
    _bindlessSupported           = supportedFeatures & GERIUM_FEATURE_BINDLESS_BIT;
    _drawIndirectSupported       = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_BIT;
    _drawIndirectCountSupported  = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT;
    _8and16BitStorageSupported   = (supportedFeatures & GERIUM_FEATURE_8_BIT_STORAGE_BIT) &&
                                 (supportedFeatures & GERIUM_FEATURE_16_BIT_STORAGE_BIT);
    _isModernPipeline = _bindlessSupported && _drawIndirectSupported && _drawIndirectCountSupported;

    // Creates an object to manage Frame Graph rendering
    check(gerium_frame_graph_create(_renderer, &_frameGraph));

    // Initializes an class for managing resources such as textures,
    // buffers, etc. And queues it for deletion using RAII and eventually
    // deletes it if the resource has not been reused.
    _resourceManager.create(_renderer, _frameGraph);

    // Adding Render Passes
    // the order of adding is not important
    addPass<PresentPass>();
    addPass<GBufferPass>();
    if (_isModernPipeline) {
        addPass<CullingPass>();
    } else {
        addPass<MergeInstancesPass>();
    }

    // Now we can load the render graph and compile it
    _resourceManager.loadFrameGraph(GRAPH_MAIN_ID);
    _resourceManager.loadFrameGraph(_isModernPipeline ? GRAPH_MODERN_ID : GRAPH_COMPAT_ID);
    check(gerium_frame_graph_compile(_frameGraph));

    // Initializing render passes
    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }

    // For rendering in this example we use multiple buffers (for the current
    // and next frames). This is a common rendering technique where the GPU
    // uses one buffer while we fill another for the next frame.
    _maxFramesInFlight = gerium_renderer_get_frames_in_flight(_renderer);
    _instances.resize(_maxFramesInFlight);
    _materials.resize(_maxFramesInFlight);

    // Create an empty 2D texture and bind it to the texture
    // array of the descriptor set
    gerium_texture_info_t info{};
    info.width            = 1;
    info.height           = 1;
    info.depth            = 1;
    info.mipmaps          = 1;
    info.format           = GERIUM_FORMAT_R8G8B8A8_UNORM;
    info.type             = GERIUM_TEXTURE_TYPE_2D;
    info.name             = "empty_texture";
    gerium_uint32_t white = 0xFF000000;
    _emptyTexture         = _resourceManager.createTexture(info, (gerium_cdata_t) &white);
    _bindlessTextures     = _resourceManager.createDescriptorSet("", true);
    for (int i = 0; i < options.descriptor_pool_elements; ++i) {
        // Bind empty textures. Bindings cannot be more than specified in the options.descriptor_pool_elements option
        gerium_renderer_bind_texture(_renderer, _bindlessTextures, BINDLESS_BINDING, i, _emptyTexture);
    }

    // Create dynamic buffers (can be efficiently filled and used every frame) for drawing and scene data
    _drawData =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "draw_data", nullptr, sizeof(DrawData), 0);
    _activeScene = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "active_scene", nullptr, sizeof(SceneData));

    // Create a descriptor set into which resources with instances data will be bind
    _instancesData = _resourceManager.createDescriptorSet("instances_data", true);
    gerium_renderer_bind_buffer(_renderer, _instancesData, 0, _drawData);
    gerium_renderer_bind_resource(_renderer, _instancesData, 3, "command_counts", false);
    gerium_renderer_bind_resource(_renderer, _instancesData, 4, "commands", false);

    // Create a descriptor set that will contain data about the scene
    // information (mainly view and projection matrices, etc.)
    _activeSceneData = _resourceManager.createDescriptorSet("scene_data", true);
    gerium_renderer_bind_buffer(_renderer, _activeSceneData, 0, _activeScene);

    // Makes storage react to creation and destruction of objects of the Renderable
    entityRegistry().storage<entt::reactive>(STORAGE_CONSTRUCT).on_construct<Renderable>();
    entityRegistry().storage<entt::reactive>(STORAGE_DESTROY).on_destroy<Renderable>();
    entityRegistry().storage<entt::reactive>(STORAGE_UPDATE_TRANSFORM).on_update<Transform>();
}

void RenderService::stop() {
    auto& storageConstruct       = entityRegistry().storage<entt::reactive>(STORAGE_CONSTRUCT);
    auto& storageDestroy         = entityRegistry().storage<entt::reactive>(STORAGE_DESTROY);
    auto& storageUpdateTransform = entityRegistry().storage<entt::reactive>(STORAGE_UPDATE_TRANSFORM);
    entityRegistry().on_construct<Renderable>().disconnect(&storageConstruct);
    entityRegistry().on_destroy<Renderable>().disconnect(&storageDestroy);
    entityRegistry().on_update<Transform>().disconnect(&storageUpdateTransform);

    _modelsDestroyed     = 0;
    _renderableDestroyed = 0;
    _modelsInCluster.clear();

    if (_renderer) {
        for (auto& renderPass : std::ranges::reverse_view(_renderPasses)) {
            renderPass->uninitialize(_frameGraph, _renderer);
        }

        _activeSceneData = {};
        _activeScene     = {};

        _instancesData               = {};
        _textures                    = {};
        _techniques                  = {};
        _dynamicMaterialsCache       = {};
        _compatDynamicMaterialsCache = {};
        _materialsTable              = {};
        _techniquesTable             = {};
        _dynamicMaterialsCount       = {};
        _staticMaterialsCount        = {};
        _dynamicInstancesCount       = {};
        _staticInstancesCount        = {};
        _materials                   = {};
        _instances                   = {};
        _compatInstances             = {};
        _compatMaterials             = {};
        _compatMeshes                = {};
        _compatSceneData             = {};
        _dynamicMaterials            = {};
        _dynamicInstances            = {};
        _drawData                    = {};

        _cluster          = {};
        _baseTechnique    = {};
        _bindlessTextures = {};
        _emptyTexture     = {};

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

    gerium_application_get_size(application().handle(), &_width, &_height);
    if (_width == 0 || _height == 0) {
        return;
    }

    _frameIndex = gerium_uint32_t(_frame++ % _maxFramesInFlight);

    _resourceManager.update(elapsedMs);
    updateCluster();
    updateActiveSceneData();
    updateDynamicInstances();
    updateInstancesData();

    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);

    if (_error) {
        std::rethrow_exception(_error);
    }
}

void RenderService::createCluster(const Cluster& cluster) {
    _cluster      = {};
    _compatMeshes = {};
    _resourceManager.update(0);

    if (!_drawIndirectCountSupported) {
        _compatMeshes = cluster.meshes;
    }

    {
        gerium_cdata_t data;
        gerium_uint32_t size;
        std::vector<Vertex> vertices;
        if (_8and16BitStorageSupported) {
            vertices.resize(cluster.vertices.size());
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
            data = (gerium_cdata_t) vertices.data();
            size = (gerium_uint32_t) (vertices.size() * sizeof(vertices[0]));
        } else {
            data = (gerium_cdata_t) cluster.vertices.data();
            size = (gerium_uint32_t) (cluster.vertices.size() * sizeof(cluster.vertices[0]));
        }
        _cluster.vertices =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "cluster_vertices", data, size, 0);
    }

    {
        _cluster.indices =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                          false,
                                          "cluster_indices",
                                          (gerium_cdata_t) cluster.primitiveIndices.data(),
                                          (gerium_uint32_t) (cluster.primitiveIndices.size() * sizeof(uint32_t)),
                                          0);
    }

    {
        gerium_cdata_t data;
        gerium_uint32_t size;
        std::vector<Mesh> meshes;
        if (_8and16BitStorageSupported) {
            meshes.resize(cluster.meshes.size());
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
            data = (gerium_cdata_t) meshes.data();
            size = (gerium_uint32_t) (meshes.size() * sizeof(meshes[0]));
        } else {
            data = (gerium_cdata_t) cluster.meshes.data();
            size = (gerium_uint32_t) (cluster.meshes.size() * sizeof(cluster.meshes[0]));
        }
        _cluster.meshes =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "cluster_meshes", data, size, 0);
    }

    _cluster.ds = _resourceManager.createDescriptorSet("cluster_ds", true);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 0, _cluster.vertices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 1, _cluster.indices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 2, _cluster.meshes);
}

void RenderService::createStaticInstances() {
    for (auto& buffer : _instances) {
        buffer = {};
    }

    for (auto& buffer : _materials) {
        buffer = {};
    }

    _compatMaterials.clear();
    _compatInstances.clear();
    _staticInstancesCount  = 0;
    _dynamicInstancesCount = 0;
    _staticMaterialsCount  = 0;
    _dynamicMaterialsCount = 0;
    _techniquesTable.clear();
    _materialsTable.clear();
    _dynamicMaterialsCache.clear();
    _compatDynamicMaterialsCache.clear();
    _techniques.clear();
    _textures.clear();

    _resourceManager.update(0);

    gerium_cdata_t materialData;
    gerium_uint32_t materialSize;
    std::vector<Material> modernMaterials;
    std::vector<MaterialNonCompressed> compatMaterials;
    std::vector<MeshInstance> instances;

    auto view = entityRegistry().view<Renderable, Transform, Static>();
    for (auto entity : view) {
        auto& renderable = view.get<Renderable>(entity);
        auto& transform  = view.get<Transform>(entity);
        if (_8and16BitStorageSupported) {
            getMaterialsAndInstances(renderable, transform, modernMaterials, instances);
        } else {
            getMaterialsAndInstances(renderable, transform, compatMaterials, instances);
        }
    }

    if (_8and16BitStorageSupported) {
        _staticMaterialsCount = (gerium_uint32_t) modernMaterials.size();
        modernMaterials.resize(modernMaterials.size() + MAX_DYNAMIC_MATERIALS);
        materialData = modernMaterials.data();
        materialSize = gerium_uint32_t(modernMaterials.size() * sizeof(modernMaterials[0]));
    } else {
        _staticMaterialsCount = (gerium_uint32_t) compatMaterials.size();
        compatMaterials.resize(compatMaterials.size() + MAX_DYNAMIC_MATERIALS);
        materialData = compatMaterials.data();
        materialSize = gerium_uint32_t(compatMaterials.size() * sizeof(compatMaterials[0]));
    }

    _staticInstancesCount = (gerium_uint32_t) instances.size();

    if (!_drawIndirectCountSupported) {
        _compatInstances = instances;
    }

    instances.resize(instances.size() + MAX_DYNAMIC_INSTANCES);
    assert(instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);

    for (size_t i = 0; i < _materials.size(); ++i) {
        const auto name = "materials_" + std::to_string(i);

        _materials[i] = _resourceManager.createBuffer(
            GERIUM_BUFFER_USAGE_STORAGE_BIT, false, { name.c_str(), name.length() }, materialData, materialSize, 0);
    }

    for (size_t i = 0; i < _instances.size(); ++i) {
        const auto name = "instances_" + std::to_string(i);

        _instances[i] = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                      false,
                                                      { name.c_str(), name.length() },
                                                      (gerium_cdata_t) instances.data(),
                                                      (gerium_uint32_t) (instances.size() * sizeof(instances[0])),
                                                      0);
    }
}

void RenderService::updateCluster() {
    auto& registry         = entityRegistry();
    auto& storageCreate    = registry.storage<entt::reactive>(STORAGE_CONSTRUCT);
    auto& storageDestroy   = registry.storage<entt::reactive>(STORAGE_DESTROY);
    auto& storageTransform = registry.storage<entt::reactive>(STORAGE_UPDATE_TRANSFORM);

    auto needReloadCluster         = false;
    auto needUpdateStaticInstances = false;
    for (auto entity : storageCreate) {
        auto& renderable    = registry.get<Renderable>(entity);
        const auto isStatic = registry.any_of<Static>(entity);
        if (isStatic) {
            needUpdateStaticInstances = true;
        }
        for (auto& mesh : renderable.meshes) {
            if (auto it = _modelsInCluster.find(mesh.model); it != _modelsInCluster.end()) {
                for (const auto& index : it->second.indices) {
                    if (mesh.node == index.nodeIndex) {
                        mesh.mesh = index.meshIndex;
                        break;
                    }
                }
            } else {
                needReloadCluster         = true;
                needUpdateStaticInstances = true;
                break;
            }
        }
        if (needReloadCluster) {
            break;
        }
    }
    storageCreate.clear();

    if (!needUpdateStaticInstances && !storageDestroy.empty()) {
        gerium_uint32_t currentStaticInstancesCount = 0;
        for (auto _ : registry.view<Renderable, Static>()) {
            ++currentStaticInstancesCount;
        }
        needUpdateStaticInstances = currentStaticInstancesCount != _staticInstancesCount;
    }

    _renderableDestroyed += (gerium_uint32_t) storageDestroy.size();
    if (_renderableDestroyed >= deletionsToCheckUpdateCluster) {
        std::set<entt::hashed_string> removedModels;
        for (const auto& [key, _] : _modelsInCluster) {
            removedModels.insert(key);
        }
        for (auto [entity, renderable] : registry.view<Renderable>().each()) {
            for (const auto& mesh : renderable.meshes) {
                if (auto it = removedModels.find(mesh.model); it != removedModels.end()) {
                    removedModels.erase(it);
                }
            }
        }
        _modelsDestroyed += (gerium_uint32_t) removedModels.size();
        _renderableDestroyed = 0;
    }
    storageDestroy.clear();

    if (_modelsDestroyed >= deletionsToCheckUpdateCluster) {
        needReloadCluster         = true;
        needUpdateStaticInstances = true;
        _modelsDestroyed          = 0;
    }
    
    if (!needUpdateStaticInstances && !storageTransform.empty()) {
        for (auto entity : storageTransform) {
            if (entityRegistry().any_of<Static>(entity)) {
                needUpdateStaticInstances = true;
                break;
            }
        }
    }
    storageTransform.clear();

    if (needReloadCluster) {
        _modelsInCluster.clear();

        Cluster cluster;
        std::map<entt::hashed_string, Model> models;
        auto getModel = [this, &cluster, &models](const entt::hashed_string& modelId) -> const Model& {
            if (auto it = models.find(modelId); it != models.end()) {
                return it->second;
            }
            models[modelId] = loadModel(cluster, modelId);
            auto& model     = models[modelId];
            auto& indices   = _modelsInCluster[modelId].indices;
            for (const auto& mesh : model.meshes) {
                indices.emplace_back(mesh.nodeIndex, mesh.meshIndex);
            }
            return model;
        };

        for (auto entity : registry.view<Renderable>()) {
            auto& renderable = registry.get<Renderable>(entity);
            auto isStatic    = registry.any_of<Static>(entity);
            for (auto& mesh : renderable.meshes) {
                const auto& model = getModel(mesh.model);
                for (const auto& reloadMesh : model.meshes) {
                    if (reloadMesh.nodeIndex == mesh.node) {
                        mesh.mesh = reloadMesh.meshIndex;
                    }
                }
            }
        }
        if (!cluster.vertices.empty()) {
            createCluster(cluster);
        }
    }

    if (needUpdateStaticInstances) {
        createStaticInstances();
    }
}

void RenderService::updateActiveSceneData() {
    auto view = entityRegistry().view<Camera>();

    Camera* camera = nullptr;
    for (auto entity : view) {
        if (auto& c = view.get<Camera>(entity); c.active) {
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

    SceneData sceneData;
    sceneData.view               = camera->view;
    sceneData.viewProjection     = camera->viewProjection;
    sceneData.prevViewProjection = camera->prevViewProjection;
    sceneData.invViewProjection  = glm::inverse(camera->viewProjection);
    sceneData.viewPosition       = glm::vec4(camera->position, 1.0f);
    sceneData.eye                = glm::vec4(camera->front, 1.0f);
    sceneData.frustum            = vec4(frustumX.x, frustumX.z, frustumY.y, frustumY.z);
    sceneData.p00p11             = p00p11;
    sceneData.farNear            = glm::vec2(camera->farPlane, camera->nearPlane);
    sceneData.invResolution      = invResolution;
    sceneData.resolution         = camera->resolution;
    sceneData.lodTarget          = (2.0f / p00p11.y) * invResolution.x;

    auto data = (SceneData*) gerium_renderer_map_buffer(_renderer, _activeScene, 0, sizeof(SceneData));
    memcpy(data, &sceneData, sizeof(SceneData));
    gerium_renderer_unmap_buffer(_renderer, _activeScene);

    if (!_drawIndirectCountSupported) {
        _compatSceneData = sceneData;
    }
}

void RenderService::updateDynamicInstances() {
    auto view = entityRegistry().view<Renderable, Transform>(entt::exclude<Static>);

    gerium_cdata_t materialData;
    gerium_uint32_t materialSize;
    std::vector<MeshInstance> instances;

    for (auto entity : view) {
        auto& renderable = view.get<Renderable>(entity);
        auto& transform  = view.get<Transform>(entity);
        if (_8and16BitStorageSupported) {
            getMaterialsAndInstances(renderable, transform, _dynamicMaterialsCache, instances);
        } else {
            getMaterialsAndInstances(renderable, transform, _compatDynamicMaterialsCache, instances);
        }
    }

    assert(_staticInstancesCount + instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);

    if (_8and16BitStorageSupported) {
        _dynamicMaterialsCount = (gerium_uint32_t) _dynamicMaterialsCache.size();
        materialData           = _dynamicMaterialsCache.data();
        materialSize           = gerium_uint32_t(_dynamicMaterialsCache.size() * sizeof(Material));
    } else {
        _dynamicMaterialsCount = (gerium_uint32_t) _compatDynamicMaterialsCache.size();
        materialData           = _compatDynamicMaterialsCache.data();
        materialSize           = gerium_uint32_t(_compatDynamicMaterialsCache.size() * sizeof(MaterialNonCompressed));
    }

    _dynamicInstancesCount = (gerium_uint32_t) instances.size();

    if (_dynamicMaterialsCount) {
        _dynamicMaterials =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "", materialData, materialSize, 0);
    }

    if (_dynamicInstancesCount) {
        _dynamicInstances = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                          true,
                                                          "",
                                                          (gerium_cdata_t) instances.data(),
                                                          (gerium_uint32_t) (instances.size() * sizeof(instances[0])),
                                                          0);
    }

    if (!_drawIndirectCountSupported) {
        _compatInstances.resize(_staticInstancesCount);
        for (const auto& instance : instances) {
            _compatInstances.push_back(instance);
        }
    }
}

void RenderService::updateInstancesData() {
    auto drawData       = (DrawData*) gerium_renderer_map_buffer(_renderer, _drawData, 0, sizeof(DrawData));
    drawData->drawCount = instancesCount();
    gerium_renderer_unmap_buffer(_renderer, _drawData);

    gerium_renderer_bind_buffer(_renderer, _instancesData, 1, _materials[_frameIndex]);
    gerium_renderer_bind_buffer(_renderer, _instancesData, 2, _instances[_frameIndex]);
}

gerium_uint64_t RenderService::materialDataHash(const MaterialData& material) noexcept {
    const auto baseColor         = material.baseColorTexture.value();
    const auto metallicRoughness = material.metallicRoughnessTexture.value();
    const auto normal            = material.normalTexture.value();
    const auto occlusion         = material.occlusionTexture.value();
    const auto emissive          = material.emissiveTexture.value();

    gerium_uint64_t seed = RAPID_SEED;

    seed = rapidhash_withSeed(&baseColor, sizeof(baseColor), seed);
    seed = rapidhash_withSeed(&metallicRoughness, sizeof(metallicRoughness), seed);
    seed = rapidhash_withSeed(&normal, sizeof(normal), seed);
    seed = rapidhash_withSeed(&occlusion, sizeof(occlusion), seed);
    seed = rapidhash_withSeed(&emissive, sizeof(emissive), seed);
    seed = rapidhash_withSeed(&material.baseColorFactor.x, sizeof(material.baseColorFactor), seed);
    seed = rapidhash_withSeed(&material.emissiveFactor.x, sizeof(material.emissiveFactor), seed);
    seed = rapidhash_withSeed(&material.metallicFactor, sizeof(material.metallicFactor), seed);
    seed = rapidhash_withSeed(&material.roughnessFactor, sizeof(material.roughnessFactor), seed);
    seed = rapidhash_withSeed(&material.occlusionStrength, sizeof(material.occlusionStrength), seed);
    seed = rapidhash_withSeed(&material.alphaCutoff, sizeof(material.alphaCutoff), seed);

    return seed;
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
