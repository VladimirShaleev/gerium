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

using namespace entt::literals;

// Merges dynamic instances buffer into the current instances buffer
void RenderService::mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer) {
    if (_dynamicMaterialsCount) {
        // Add dynamic materials to the end of the current static material buffer
        const auto size = _8and16BitStorageSupported ? sizeof(Material) : sizeof(MaterialNonCompressed);
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _dynamicMaterials,
                                          0,
                                          _materials[_frameIndex],
                                          _staticMaterialsCount * size,
                                          _dynamicMaterialsCount * size);
    }
    if (_dynamicInstancesCount) {
        // Add dynamic instances to the end of the current static instance buffer
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _dynamicInstances,
                                          0,
                                          _instances[_frameIndex],
                                          _staticInstancesCount * sizeof(MeshInstance),
                                          _dynamicInstancesCount * sizeof(MeshInstance));
    }
}

void RenderService::start() {
    // We will explicitly set some parameters necessary, which are sufficient
    // for this example. If the option is not set, the default value will be used
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
    check(gerium_profiler_create(_renderer, &_profiler)); // in this example does not use

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
    // (the order of adding is not important)
    addPass<PresentPass>(); // pass for draw to swapchain framebuffer and other post effects
    addPass<GBufferPass>(); // pass to create GBuffer of all opaque geometry
    if (_isModernPipeline) {
        addPass<CullingPass>(); // add pass with GPU frustum culling
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
    entityRegistry().storage<entt::reactive>(STORAGE_CONSTRUCT).on_construct<Renderable>().on_update<Renderable>();
    entityRegistry().storage<entt::reactive>(STORAGE_DESTROY).on_destroy<Renderable>().on_destroy<Static>();
    entityRegistry()
        .storage<entt::reactive>(STORAGE_UPDATE)
        .on_construct<Static>()
        .on_construct<Transform>()
        .on_update<Transform>()
        .on_destroy<Transform>();
}

void RenderService::stop() {
    // Remove react storages
    auto& storageConstruct = entityRegistry().storage<entt::reactive>(STORAGE_CONSTRUCT);
    auto& storageDestroy   = entityRegistry().storage<entt::reactive>(STORAGE_DESTROY);
    auto& storageUpdate    = entityRegistry().storage<entt::reactive>(STORAGE_UPDATE);
    entityRegistry().on_construct<Renderable>().disconnect(&storageConstruct);
    entityRegistry().on_update<Renderable>().disconnect(&storageConstruct);
    entityRegistry().on_destroy<Renderable>().disconnect(&storageDestroy);
    entityRegistry().on_destroy<Static>().disconnect(&storageDestroy);
    entityRegistry().on_construct<Static>().disconnect(&storageUpdate);
    entityRegistry().on_construct<Transform>().disconnect(&storageUpdate);
    entityRegistry().on_update<Transform>().disconnect(&storageUpdate);
    entityRegistry().on_destroy<Transform>().disconnect(&storageUpdate);

    if (_renderer) {
        for (auto& renderPass : std::ranges::reverse_view(_renderPasses)) {
            renderPass->uninitialize(_frameGraph, _renderer);
        }

        // Cleaning up data vectors to support legacy systems
        _compatInstances = {};
        _compatMaterials = {};
        _compatMeshes    = {};
        _compatSceneData = {};

        // Cleaning of materials (techniques, textures and cache tables)
        _textures               = {};
        _materialsNonCompressed = {};
        _materialsCompressed    = {};
        _materialsTable         = {};
        _techniquesTable        = {};
        _techniques             = {};

        // Cleaning up instances
        _dynamicMaterialsCount = {};
        _staticMaterialsCount  = {};
        _dynamicInstancesCount = {};
        _staticInstancesCount  = {};
        _instancesData         = {};
        _materials             = {};
        _instances             = {};
        _dynamicMaterials      = {};
        _dynamicInstances      = {};
        _drawData              = {};

        // Scene data cleaning (camera)
        _activeSceneData = {};
        _activeScene     = {};

        // Zeroing of counters
        _modelsDestroyed     = 0;
        _renderableDestroyed = 0;

        // Cleaning GPU geometric data of a cluster
        _modelsInCluster.clear();
        _cluster = {};

        // Cleaning up the bindless texture set and the stub texture
        _bindlessTextures = {};
        _emptyTexture     = {};

        // Clear all GPU resources that are no longer in use
        _resourceManager.destroy();

        // Destroy frame graph object
        if (_frameGraph) {
            gerium_frame_graph_destroy(_frameGraph);
            _frameGraph = nullptr;
        }

        // Destroy profiler object
        if (_profiler) {
            gerium_profiler_destroy(_profiler);
            _profiler = nullptr;
        }

        // Destroy renderer object
        gerium_renderer_destroy(_renderer);
        _renderer = nullptr;

        _error = nullptr; // remove error
    }
}

void RenderService::update(gerium_uint64_t elapsedMs, gerium_float64_t /* elapsed */) {
    // Let's start a new frame, if the rendering system returned GERIUM_RESULT_SKIP_FRAME,
    // then rendering is not possible now. For example, if the window is minimized, etc.
    // In these cases, services can still receive an update (for example, if this is
    // a network game and you need to do some logic, but you don't need to draw).
    if (gerium_renderer_new_frame(_renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return;
    }

    // Get the current size of the render area
    gerium_application_get_size(application().handle(), &_width, &_height);

    // Increment frame number and update frame index
    _frameIndex = gerium_uint32_t(_frame++ % _maxFramesInFlight);

    // Remove unused GPU resources
    _resourceManager.update(elapsedMs);

    updateCluster();          // if the cluster/statics have changed, update the GPU cluster/static
    updateActiveSceneData();  // update GPU scene data (camera)
    updateDynamicInstances(); // update dynamic instance
    updateInstancesData();    // Set bindings for instances data to descriptor set

    // Run frame graph passes attached to _renderer to render a frame
    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);

    if (_error) {
        // In case of an unhandled exception, we terminate the service
        std::rethrow_exception(_error);
    }
}

// We have loaded the cluster data (in linear form) and this method will
// fill the GPU SSBO buffers (and pack the data if possible)
void RenderService::createCluster(const Cluster& cluster) {
    // Remove references to old GPU data
    _cluster = {};
    _resourceManager.update(0); // and free up unused resources

    // If indirect rendering cannot pass the number of draws to the SSBO
    // (GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT is not supported), then
    // we store the meshes on the CPU side.
    if (!_drawIndirectCountSupported) {
        _compatMeshes = cluster.meshes;
    }

    // Fill the GPU buffer in the fastest memory with vertex data
    {
        gerium_cdata_t data;
        gerium_uint32_t size;
        std::vector<Vertex> vertices;
        if (_8and16BitStorageSupported) {
            // If it supports 8-bit and 16-bit storage, then we pack the data
            vertices.resize(cluster.vertices.size());
            for (size_t i = 0; i < cluster.vertices.size(); ++i) {
                const auto& v = cluster.vertices[i];
                const auto n  = glm::vec3(v.nx, v.ny, v.nz) * 127.0f + 127.5f;
                const auto t  = glm::vec3(v.tx, v.ty, v.tz) * 127.0f + 127.5f;
                const auto s  = v.ts < 0.0f ? -1 : 1;

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

    // Fill the GPU buffer in the fastest memory with index data
    {
        _cluster.indices =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                          false,
                                          "cluster_indices",
                                          (gerium_cdata_t) cluster.primitiveIndices.data(),
                                          (gerium_uint32_t) (cluster.primitiveIndices.size() * sizeof(uint32_t)),
                                          0);
    }

    // Fill the GPU buffer in the fastest memory with mesh data
    // (vertex offset/count, bbox, lods info ets)
    {
        gerium_cdata_t data;
        gerium_uint32_t size;
        std::vector<Mesh> meshes;
        if (_8and16BitStorageSupported) {
            // If it supports 8-bit and 16-bit storage, then we pack the data
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

    // Create a descriptor set (in this case, global, which means that its bindings
    // will not change every frame. This will allow for a more optimal allocation
    // of the descriptor set from the pool). And bind the current cluster data to it.
    _cluster.ds = _resourceManager.createDescriptorSet("cluster_ds", true);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 0, _cluster.vertices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 1, _cluster.indices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 2, _cluster.meshes);
}

// Filling SSBO GPU buffers for static instances and their materials.
// Static instances and materials do not change, meaning that their
// allocation can be done once at boot
void RenderService::createStaticInstances() {
    // clear previous instances
    for (auto& buffer : _instances) {
        buffer = {};
    }

    // clear previous materials
    for (auto& buffer : _materials) {
        buffer = {};
    }

    _staticInstancesCount  = 0;
    _dynamicInstancesCount = 0;
    _staticMaterialsCount  = 0;
    _dynamicMaterialsCount = 0;

    // Remove references to old GPU data
    _techniques.clear();
    _textures.clear();
    _techniquesTable.clear();
    _materialsTable.clear();
    _materialsCompressed.clear();
    _materialsNonCompressed.clear();
    _compatMaterials.clear();
    _compatInstances.clear();

    // and free up unused resources
    _resourceManager.update(0);

    // Get static instances for meshes. getInstances also indexes the materials
    // used by these instances and fill _materialsCompressed (or
    // _materialsNonCompressed if packing is not supported)
    std::vector<MeshInstance> instances = getInstances(true);

    // Get the address of the materials and the size of the entire material buffer in bytes
    gerium_cdata_t materialData;
    gerium_uint32_t materialSize;
    if (_8and16BitStorageSupported) {
        _staticMaterialsCount = (gerium_uint32_t) _materialsCompressed.size(); // read below *
        _materialsCompressed.resize(_materialsCompressed.size() + MAX_DYNAMIC_MATERIALS);
        materialData = _materialsCompressed.data();
        materialSize = gerium_uint32_t(_materialsCompressed.size() * sizeof(_materialsCompressed[0]));
    } else {
        _staticMaterialsCount = (gerium_uint32_t) _materialsNonCompressed.size(); // read below *
        _materialsNonCompressed.resize(_materialsNonCompressed.size() + MAX_DYNAMIC_MATERIALS);
        materialData = _materialsNonCompressed.data();
        materialSize = gerium_uint32_t(_materialsNonCompressed.size() * sizeof(_materialsNonCompressed[0]));
    }

    // Save current number of static instances
    _staticInstancesCount = (gerium_uint32_t) instances.size();

    // If indirect rendering cannot pass the draw count to SSBO
    // (GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT is not supported),
    // then instances are also stored on the CPU side
    if (!_drawIndirectCountSupported) {
        _compatInstances = instances;
    }

    // Increase the size of the buffer with instances, just to immediately
    // fill the SSBO buffer on the GPU with one call (including extra space for dynamic data)
    // * Explanation: in fact, the same SSBO buffer will be used for rendering for static and
    // dynamic geometry. Every frame we simply copy the dynamic data after the static.
    instances.resize(instances.size() + MAX_DYNAMIC_INSTANCES);
    assert(instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);

    // We create a fairly large SSBO buffer on the GPU side (for both statics and dynamics). At
    // the same time, we copy static data. Several SSBO buffers are created (_maxFramesInFlight).
    // This is necessary so that while the frame with _materials[0] is being drawn, we can fill
    // in dynamic data in _materials[1] and so on in a circle.
    for (size_t i = 0; i < _materials.size(); ++i) {
        const auto name = "materials_" + std::to_string(i);
        _materials[i]   = _resourceManager.createBuffer(
            GERIUM_BUFFER_USAGE_STORAGE_BIT, false, { name.c_str(), name.length() }, materialData, materialSize, 0);
    }

    // We create a fairly large SSBO buffer on the GPU side (for both statics and dynamics). At
    // the same time, we copy static data. Several SSBO buffers are created (_maxFramesInFlight).
    // This is necessary so that while the frame with _instances[0] is being drawn, we can fill
    // in dynamic data in _instances[1] and so on in a circle.
    for (size_t i = 0; i < _instances.size(); ++i) {
        const auto name = "instances_" + std::to_string(i);
        _instances[i]   = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                      false,
                                                        { name.c_str(), name.length() },
                                                      (gerium_cdata_t) instances.data(),
                                                      (gerium_uint32_t) (instances.size() * sizeof(instances[0])),
                                                      0);
    }

    // The CPU side material data is no longer needed, as we have already copied it to the GPU.
    _materialsCompressed.clear();
    _materialsNonCompressed.clear();
}

// This method checks whether the entire cluster needs to be updated (and
// starts recreating it if necessary). It also checks whether static data
// needs to be updated (and recreates it if necessary)
void RenderService::updateCluster() {
    auto& registry       = entityRegistry();
    auto& storageCreate  = registry.storage<entt::reactive>(STORAGE_CONSTRUCT);
    auto& storageDestroy = registry.storage<entt::reactive>(STORAGE_DESTROY);
    auto& storageUpdate  = registry.storage<entt::reactive>(STORAGE_UPDATE);

    // This method is called every frame and by default we assume that
    // neither the cluster nor the statics need to be updated.
    auto needReloadCluster         = false;
    auto needUpdateStaticInstances = false;

    // Get new or updated Renderable's from the reactive store
    for (auto entity : storageCreate) {
        auto& renderable    = registry.get<Renderable>(entity);
        const auto isStatic = registry.any_of<Static>(entity);
        if (isStatic) {
            // if the entity also has a static component, then the statics need to be updated
            // In the game itself (after the level is created by the developer), this will
            // naturally not happen. During gameplay, only dynamics will appear, change and be destroyed
            needUpdateStaticInstances = true;
        }
        // We check the availability of data for meshes in the current cluster loaded in the GPU
        for (auto& mesh : renderable.meshes) {
            // If there is a mesh in a cluster, take its index in this cluster and save it in the
            // meshes[n] field of the Renderable component.
            if (auto it = _modelsInCluster.find(mesh.model); it != _modelsInCluster.end()) {
                for (const auto& index : it->second.indices) {
                    if (mesh.node == index.nodeIndex) {
                        mesh.mesh = index.meshIndex;
                        break;
                    }
                }
            } else {
                // If there is no grid in the GPU, then need to update the entire
                // cluster and all the statics.
                // Naturally, this will not happen in the game itself (after the level is
                // created by the developer). During gameplay, when loading a level, we will
                // already have all the components of Renderable and we will build a cluster and static once.
                needReloadCluster         = true;
                needUpdateStaticInstances = true;
                break;
            }
        }
        // If you need to update the cluster, then you don't need to check the components anymore.
        if (needReloadCluster) {
            break;
        }
    }
    storageCreate.clear();

    if (!needUpdateStaticInstances && !storageDestroy.empty()) {
        // If the Renderable and/or Static components have been removed, we calculate the
        // current amount of rendered static.
        gerium_uint32_t currentStaticInstancesCount = 0;
        for (auto _ : registry.view<Renderable, Static>()) {
            ++currentStaticInstancesCount;
        }
        // If the number of statics does not match the current statics in the registry,
        // then we need to update the statics.
        needUpdateStaticInstances = currentStaticInstancesCount != _staticInstancesCount;
    }

    // Counting the deleted number of components. And if a certain limit is reached...
    _renderableDestroyed += (gerium_uint32_t) storageDestroy.size();
    if (_renderableDestroyed >= deletionsToCheckUpdateCluster) {
        // we are looking for remote models (which are in the cluster but are no longer in use)
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
        // If the number of deleted models has exceeded a certain limit, then
        // need to rebuild the cluster without them, as well as update the statics.
        needReloadCluster         = true;
        needUpdateStaticInstances = true;
        _modelsDestroyed          = 0;
    }

    // If there have been changes in entities with static (add Static or
    // add/update/remove Transform), then you need to update the static.
    if (!needUpdateStaticInstances && !storageUpdate.empty()) {
        for (auto entity : storageUpdate) {
            if (entityRegistry().any_of<Static>(entity)) {
                needUpdateStaticInstances = true;
                break;
            }
        }
    }
    storageUpdate.clear();

    // If you need to update the cluster, we will prepare the data for this.
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

        // We go through all the Renderable entities
        for (auto entity : registry.view<Renderable>()) {
            auto& renderable = registry.get<Renderable>(entity);
            for (auto& mesh : renderable.meshes) { // go through all its meshes
                // getModel loads the model if it is not loaded yet (gets the model
                // meta and adds the mesh data to the cluster)
                const auto& model = getModel(mesh.model);
                for (const auto& reloadMesh : model.meshes) {
                    if (reloadMesh.nodeIndex == mesh.node) {
                        // Update the grid index to match the index in the loaded cluster
                        mesh.mesh = reloadMesh.meshIndex;
                    }
                }
            }
        }
        if (!cluster.vertices.empty()) {
            // If the resulting cluster contains data, then we create a cluster in the GPU
            // cluster is the data in a linear form of all geometry on the CPU side. After
            // the creation of the SSBO buffers, the data on the CPU side is no longer needed.
            createCluster(cluster);
        }
    }

    if (needUpdateStaticInstances) {
        // Rebuilding statics. This means that will recreate the SSBO
        // buffers for materials and instances
        createStaticInstances();
    }
}

// Updates the _activeScene UBO buffer for the camera
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

    // If GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT is not supported, then we
    // also save the data on the CPU side
    if (!_drawIndirectCountSupported) {
        _compatSceneData = sceneData;
    }
}

// Updates dynamic instances, fills dynamic GPU buffers (optimal for writing every frame
// from the CPU side) with materials and instances
void RenderService::updateDynamicInstances() {
    gerium_cdata_t materialData;
    gerium_uint32_t materialSize;
    std::vector<MeshInstance> instances = getInstances(false); // get dynamic instances

    assert(_staticInstancesCount + instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);
    _dynamicInstancesCount = (gerium_uint32_t) instances.size(); // save the number of dynamic instances

    if (_8and16BitStorageSupported) {
        _dynamicMaterialsCount = (gerium_uint32_t) _materialsCompressed.size();
        materialData           = _materialsCompressed.data();
        materialSize           = gerium_uint32_t(_materialsCompressed.size() * sizeof(Material));
    } else {
        _dynamicMaterialsCount = (gerium_uint32_t) _materialsNonCompressed.size();
        materialData           = _materialsNonCompressed.data();
        materialSize           = gerium_uint32_t(_materialsNonCompressed.size() * sizeof(MaterialNonCompressed));
    }

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

    // If GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT is not supported, then we
    // also save the instances on the CPU side (after static)
    if (!_drawIndirectCountSupported) {
        _compatInstances.resize(_staticInstancesCount + _dynamicInstancesCount);
        for (gerium_uint32_t i = 0; i < _dynamicInstancesCount; ++i) {
            _compatInstances[_staticInstancesCount + i] = instances[i];
        }
    }

    // This time we do not clear _materialsCompressed and _materialsNonCompressed, as they are needed to update
    // the dynamics every frame. When adding new dynamic materials, they will be added to one of these buffers.
}

// Update data for the camera and re-bind the SSBO for materials and instances to the destriptor set
void RenderService::updateInstancesData() {
    auto drawData       = (DrawData*) gerium_renderer_map_buffer(_renderer, _drawData, 0, sizeof(DrawData));
    drawData->drawCount = instancesCount();
    gerium_renderer_unmap_buffer(_renderer, _drawData);

    gerium_renderer_bind_buffer(_renderer, _instancesData, 1, _materials[_frameIndex]);
    gerium_renderer_bind_buffer(_renderer, _instancesData, 2, _instances[_frameIndex]);
}

// Returns static or dynamic instances
std::vector<MeshInstance> RenderService::getInstances(bool isStatic) {
    std::vector<MeshInstance> instances;
    auto emplace = [this, &instances](auto&& view) {
        for (auto entity : view) {
            auto& renderable = view.get<Renderable>(entity);
            auto& transform  = view.get<Transform>(entity);
            emplaceInstance(renderable, transform, instances);
        }
    };
    if (isStatic) {
        emplace(entityRegistry().view<Renderable, Transform, Static>());
    } else {
        emplace(entityRegistry().view<Renderable, Transform>(entt::exclude<Static>));
    }
    return instances;
}

void RenderService::emplaceInstance(const Renderable& renderable,
                                    const Transform& transform,
                                    std::vector<MeshInstance>& instances) {
    for (const auto& meshData : renderable.meshes) {
        instances.push_back({});
        auto& instance        = instances.back();
        instance.world        = transform.matrix;
        instance.prevWorld    = transform.prevMatrix;
        instance.normalMatrix = glm::transpose(glm::inverse(transform.matrix));
        instance.scale        = glm::max(glm::max(transform.scale.x, transform.scale.y), transform.scale.z);
        instance.mesh         = meshData.mesh;
        instance.technique    = getOrEmplaceTechnique(meshData.material);
        instance.material     = getOrEmplaceMaterial(meshData.material);
    }
}

// Loading and indexing of technique
gerium_uint32_t RenderService::getOrEmplaceTechnique(const MaterialData& material) {
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
    return techniqueIndex;
}

// Loading and indexing of material
gerium_uint32_t RenderService::getOrEmplaceMaterial(const MaterialData& material) {
    const auto hash = materialDataHash(material);

    gerium_uint32_t materialIndex = 0;
    if (auto it = _materialsTable.find(hash); it != _materialsTable.end()) {
        materialIndex = it->second;
    } else {
        materialIndex = (gerium_uint32_t) _materialsTable.size();
        _materialsTable.insert({ hash, materialIndex });

        auto loadTexture = [this](const entt::hashed_string& name) -> gerium_uint16_t {
            if (name.size()) {
                Texture result = _resourceManager.loadTexture(name);
                _textures.insert(result);
                gerium_texture_h texture = result;
                gerium_renderer_bind_texture(_renderer, _bindlessTextures, BINDLESS_BINDING, texture.index, texture);
                return texture.index;
            }
            gerium_texture_h emptyTexture = _emptyTexture;
            return emptyTexture.index;
        };

        auto emplaceMaterial = [this, &loadTexture](auto& materials, const MaterialData& material, auto pred) {
            materials.push_back({});
            auto& mat                    = materials.back();
            mat.baseColorFactor[0]       = pred(material.baseColorFactor.x);
            mat.baseColorFactor[1]       = pred(material.baseColorFactor.y);
            mat.baseColorFactor[2]       = pred(material.baseColorFactor.z);
            mat.baseColorFactor[3]       = pred(material.baseColorFactor.w);
            mat.emissiveFactor[0]        = pred(material.emissiveFactor.x);
            mat.emissiveFactor[1]        = pred(material.emissiveFactor.y);
            mat.emissiveFactor[2]        = pred(material.emissiveFactor.z);
            mat.metallicFactor           = pred(material.metallicFactor);
            mat.roughnessFactor          = pred(material.roughnessFactor);
            mat.occlusionStrength        = pred(material.occlusionStrength);
            mat.alphaCutoff              = pred(material.alphaCutoff);
            mat.baseColorTexture         = loadTexture(material.baseColorTexture);
            mat.metallicRoughnessTexture = loadTexture(material.metallicRoughnessTexture);
            mat.normalTexture            = loadTexture(material.normalTexture);
            mat.occlusionTexture         = loadTexture(material.occlusionTexture);
            mat.emissiveTexture          = loadTexture(material.emissiveTexture);
            return mat;
        };

        if (_8and16BitStorageSupported) {
            emplaceMaterial(_materialsCompressed, material, meshopt_quantizeHalf);
        } else {
            emplaceMaterial(_materialsNonCompressed, material, [](const auto value) {
                return value;
            });
        }

        // If GERIUM_FEATURE_BINDLESS_BIT is not supported, then we also need to store all
        // materials on the CPU side in _compatMaterials to do texture bindings for each material when rendering
        if (!_bindlessSupported) {
            emplaceMaterial(_compatMaterials, material, [](const auto value) {
                return value;
            });
        }
    }
    return materialIndex;
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

// The C callback function to wrap a C++ method
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

// The C callback function to wrap a C++ method
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

// The C callback function to wrap a C++ method
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
