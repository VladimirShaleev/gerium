#include "RenderService.hpp"
#include "../Application.hpp"
#include "../Model.hpp"
#include "../components/Camera.hpp"
#include "../components/Static.hpp"
#include "../passes/CullingPass.hpp"
#include "../passes/GBufferPass.hpp"
#include "../passes/PresentPass.hpp"
#include "SceneService.hpp"

#include <ranges>

ResourceManager& RenderService::resourceManager() noexcept {
    return _resourceManager;
}

gerium_uint64_t RenderService::frame() const noexcept {
    return _frame;
}

gerium_uint32_t RenderService::frameIndex() const noexcept {
    return _frameIndex;
}

gerium_technique_h RenderService::baseTechnique() const noexcept {
    return _baseTechnique;
}

gerium_descriptor_set_h RenderService::sceneData() const noexcept {
    return _activeSceneData;
}

gerium_descriptor_set_h RenderService::clusterData() const noexcept {
    return _cluster.ds;
}

gerium_descriptor_set_h RenderService::instancesData() const noexcept {
    return _instancesData;
}

gerium_descriptor_set_h RenderService::textures() const noexcept {
    return _bindlessTextures;
}

gerium_uint32_t RenderService::instancesCount() const noexcept {
    return _staticInstancesCount + _dynamicInstancesCount;
}

const std::vector<Technique>& RenderService::techniques() const noexcept {
    return _techniques;
}

void RenderService::createCluster(const Cluster& cluster) {
    if (!_8and16BitStorageSupported) {
        throw std::runtime_error("Support for 8-bit and 16-bit storage is currently mandatory");
    }

    _cluster = {};

    _resourceManager.update(0);

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

        _cluster.vertices = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                          false,
                                                          "cluster_vertices",
                                                          (gerium_cdata_t) vertices.data(),
                                                          (gerium_uint32_t) (vertices.size() * sizeof(Vertex)),
                                                          0);
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

        _cluster.meshes = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                        false,
                                                        "cluster_meshes",
                                                        (gerium_cdata_t) meshes.data(),
                                                        (gerium_uint32_t) (meshes.size() * sizeof(Mesh)),
                                                        0);
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

    _staticInstancesCount  = 0;
    _dynamicInstancesCount = 0;
    _staticMaterialsCount  = 0;
    _dynamicMaterialsCount = 0;
    _techniquesTable.clear();
    _materialsTable.clear();
    _dynamicMaterialsCache.clear();
    _techniques.clear();
    _textures.clear();

    _resourceManager.update(0);

    auto view = entityRegistry().view<Renderable, WorldTransform, Static>();

    std::vector<Material> materials;
    std::vector<MeshInstance> instances;

    for (auto entity : view) {
        auto& renderable     = view.get<Renderable>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);
        getMaterialsAndInstances(renderable, worldTransform, materials, instances);
    }

    _staticMaterialsCount = (gerium_uint32_t) materials.size();
    _staticInstancesCount = (gerium_uint32_t) instances.size();

    materials.resize(materials.size() + MAX_DYNAMIC_MATERIALS);
    instances.resize(instances.size() + MAX_DYNAMIC_INSTANCES);

    assert(instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);

    for (size_t i = 0; i < _materials.size(); ++i) {
        const auto name = "materials_" + std::to_string(i);

        _materials[i] = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                      false,
                                                      { name.c_str(), name.length() },
                                                      (gerium_cdata_t) materials.data(),
                                                      (gerium_uint32_t) (materials.size() * sizeof(materials[0])),
                                                      0);
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

void RenderService::mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer) {
    if (_dynamicMaterialsCount) {
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _dynamicMaterials,
                                          0,
                                          _materials[_frameIndex],
                                          _staticMaterialsCount * sizeof(Material),
                                          _dynamicMaterialsCount * sizeof(Material));
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
    _resourceManager.loadTechnique(TECH_OTHER_ID); // TODO: remove later

    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }

    _maxFramesInFlight = gerium_renderer_get_frames_in_flight(_renderer);
    _instances.resize(_maxFramesInFlight);
    _materials.resize(_maxFramesInFlight);

    _drawData = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "instances_count", nullptr, sizeof(DrawData), 0);

    _activeScene = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "active_scene", nullptr, sizeof(SceneData));

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

    _bindlessTextures = _resourceManager.createDescriptorSet("", true);
    for (int i = 0; i < MAX_INSTANCES_PER_TECHNIQUE; ++i) {
        gerium_renderer_bind_texture(_renderer, _bindlessTextures, BINDLESS_BINDING, i, _emptyTexture);
    }

    _instancesData = _resourceManager.createDescriptorSet("instances_data", true);
    gerium_renderer_bind_buffer(_renderer, _instancesData, 0, _drawData);
    gerium_renderer_bind_resource(_renderer, _instancesData, 3, "command_counts", false);
    gerium_renderer_bind_resource(_renderer, _instancesData, 4, "commands", false);

    _activeSceneData = _resourceManager.createDescriptorSet("scene_data", true);
    gerium_renderer_bind_buffer(_renderer, _activeSceneData, 0, _activeScene);
}

void RenderService::stop() {
    if (_renderer) {
        for (auto& renderPass : std::ranges::reverse_view(_renderPasses)) {
            renderPass->uninitialize(_frameGraph, _renderer);
        }

        _activeSceneData = {};
        _activeScene     = {};

        _instancesData         = {};
        _textures              = {};
        _techniques            = {};
        _dynamicMaterialsCache = {};
        _materialsTable        = {};
        _techniquesTable       = {};
        _dynamicMaterialsCount = {};
        _staticMaterialsCount  = {};
        _dynamicInstancesCount = {};
        _staticInstancesCount  = {};
        _materials             = {};
        _instances             = {};
        _dynamicMaterials      = {};
        _dynamicInstances      = {};
        _drawData              = {};

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
    updateActiveSceneData();
    updateDynamicInstances();
    updateInstancesData();

    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);

    if (_error) {
        std::rethrow_exception(_error);
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

    auto sceneData            = (SceneData*) gerium_renderer_map_buffer(_renderer, _activeScene, 0, sizeof(SceneData));
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
    gerium_renderer_unmap_buffer(_renderer, _activeScene);
}

void RenderService::updateDynamicInstances() {
    auto view = entityRegistry().view<Renderable, WorldTransform>(entt::exclude<Static>);

    std::vector<MeshInstance> instances;

    for (auto entity : view) {
        auto& renderable     = view.get<Renderable>(entity);
        auto& worldTransform = view.get<WorldTransform>(entity);
        getMaterialsAndInstances(renderable, worldTransform, _dynamicMaterialsCache, instances);
    }

    assert(_staticInstancesCount + instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);

    _dynamicMaterialsCount = (gerium_uint32_t) _dynamicMaterialsCache.size();
    _dynamicInstancesCount = (gerium_uint32_t) instances.size();

    if (_dynamicMaterialsCount) {
        _dynamicMaterials = _resourceManager.createBuffer(
            GERIUM_BUFFER_USAGE_STORAGE_BIT,
            true,
            "",
            (gerium_cdata_t) _dynamicMaterialsCache.data(),
            (gerium_uint32_t) (_dynamicMaterialsCache.size() * sizeof(_dynamicMaterialsCache[0])),
            0);
    }

    if (_dynamicInstancesCount) {
        _dynamicInstances = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                          true,
                                                          "",
                                                          (gerium_cdata_t) instances.data(),
                                                          (gerium_uint32_t) (instances.size() * sizeof(instances[0])),
                                                          0);
    }
}

void RenderService::updateInstancesData() {
    auto drawData       = (DrawData*) gerium_renderer_map_buffer(_renderer, _drawData, 0, sizeof(DrawData));
    drawData->drawCount = instancesCount();
    gerium_renderer_unmap_buffer(_renderer, _drawData);

    gerium_renderer_bind_buffer(_renderer, _instancesData, 1, _materials[_frameIndex]);
    gerium_renderer_bind_buffer(_renderer, _instancesData, 2, _instances[_frameIndex]);
}

void RenderService::getMaterialsAndInstances(const Renderable& renderable,
                                             const WorldTransform& worldTransform,
                                             std::vector<Material>& materials,
                                             std::vector<MeshInstance>& instances) {
    for (const auto& meshData : renderable.meshes) {
        const auto [techniqueIndex, materialIndex] = getMaterial(meshData.material, materials);

        instances.push_back({});
        auto& instance = instances.back();

        instance.world        = worldTransform.matrix;
        instance.prevWorld    = worldTransform.prevMatrix;
        instance.normalMatrix = glm::transpose(glm::inverse(worldTransform.matrix));
        instance.scale        = worldTransform.scale;
        instance.mesh         = meshData.mesh;
        instance.technique    = techniqueIndex;
        instance.material     = materialIndex;
    }
}

std::pair<gerium_uint32_t, gerium_uint32_t> RenderService::getMaterial(const MaterialData& material,
                                                                       std::vector<Material>& result) {
    gerium_uint32_t techniqueIndex;
    if (auto it = _techniquesTable.find(material.name); it != _techniquesTable.end()) {
        techniqueIndex = it->second;
    } else {
        auto technique = _resourceManager.loadTechnique(material.name);

        techniqueIndex = (gerium_uint32_t) _techniques.size();
        _techniques.push_back(technique);
        _techniquesTable.insert({ material.name, techniqueIndex });

        assert(_techniques.size() < MAX_TECHNIQUES);
    }

    const auto hash = materialDataHash(material);

    gerium_uint32_t materialIndex = 0;
    if (auto it = _materialsTable.find(hash); it != _materialsTable.end()) {
        materialIndex = it->second;
    } else {
        materialIndex = (gerium_uint32_t) _materialsTable.size();
        _materialsTable.insert({ hash, materialIndex });
        result.push_back({});
        auto& mat = result.back();

        auto loadTexture = [this](const entt::hashed_string& name) -> gerium_uint16_t {
            if (name.size()) {
                Texture result = _resourceManager.loadTexture(name);
                _textures.insert(result);
                gerium_texture_h texture = result;
                gerium_renderer_bind_texture(_renderer, _bindlessTextures, BINDLESS_BINDING, texture.index, texture);
                return texture.index;
            }
            return { UndefinedHandle };
        };

        mat.baseColorFactor[0]       = meshopt_quantizeHalf(material.baseColorFactor.x);
        mat.baseColorFactor[1]       = meshopt_quantizeHalf(material.baseColorFactor.y);
        mat.baseColorFactor[2]       = meshopt_quantizeHalf(material.baseColorFactor.z);
        mat.baseColorFactor[3]       = meshopt_quantizeHalf(material.baseColorFactor.w);
        mat.emissiveFactor[0]        = meshopt_quantizeHalf(material.emissiveFactor.x);
        mat.emissiveFactor[1]        = meshopt_quantizeHalf(material.emissiveFactor.y);
        mat.emissiveFactor[2]        = meshopt_quantizeHalf(material.emissiveFactor.z);
        mat.metallicFactor           = meshopt_quantizeHalf(material.metallicFactor);
        mat.roughnessFactor          = meshopt_quantizeHalf(material.roughnessFactor);
        mat.occlusionStrength        = meshopt_quantizeHalf(material.occlusionStrength);
        mat.alphaCutoff              = meshopt_quantizeHalf(material.alphaCutoff);
        mat.baseColorTexture         = loadTexture(material.baseColorTexture);
        mat.metallicRoughnessTexture = loadTexture(material.metallicRoughnessTexture);
        mat.normalTexture            = loadTexture(material.normalTexture);
        mat.occlusionTexture         = loadTexture(material.occlusionTexture);
        mat.emissiveTexture          = loadTexture(material.emissiveTexture);
    }

    return { techniqueIndex, materialIndex };
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
