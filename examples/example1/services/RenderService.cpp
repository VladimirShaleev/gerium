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

// Merges dynamic instances and materials into the current static buffers.
// This is necessary to combine static and dynamic data into a single buffer for rendering.
void RenderService::mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer) {
    if (_dynamicMaterialsCount) {
        // If there are dynamic materials, copy them to the end of the static materials buffer.
        // The size of each material depends on whether 8-bit and 16-bit storage is supported.
        const auto size = _8and16BitStorageSupported ? sizeof(Material) : sizeof(MaterialNonCompressed);
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _dynamicMaterials,
                                          0,
                                          _materials[_frameIndex],
                                          _staticMaterialsCount * size,
                                          _dynamicMaterialsCount * size);
    }
    if (_dynamicInstancesCount) {
        // If there are dynamic instances, copy them to the end of the static instances buffer.
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _dynamicInstances,
                                          0,
                                          _instances[_frameIndex],
                                          _staticInstancesCount * sizeof(MeshInstance),
                                          _dynamicInstancesCount * sizeof(MeshInstance));
    }
}

// Initializes the RenderService, setting up the renderer, frame graph, and resources.
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
    options.debug_mode = true; // Enable debug mode (validation layers, GPU object names, and logs)
#endif

    // Requested GPU features for this example:
    // - Indirect draw calls
    // - Indirect draw calls with count
    // - 8-bit and 16-bit storage
    // - Bindless textures
    constexpr auto features = GERIUM_FEATURE_DRAW_INDIRECT_BIT | GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT |
                              GERIUM_FEATURE_8_BIT_STORAGE_BIT | GERIUM_FEATURE_16_BIT_STORAGE_BIT |
                              GERIUM_FEATURE_BINDLESS_BIT;

    // Initialize the renderer with the requested features and options.
    check(gerium_renderer_create(application().handle(), features, &options, &_renderer));

    // Enable the profiler to collect GPU performance metrics.
    gerium_renderer_set_profiler_enable(_renderer, true);
    check(gerium_profiler_create(_renderer, &_profiler)); // Profiler is not used in this example

    // Check which requested features are supported by the GPU.
    const auto supportedFeatures = gerium_renderer_get_enabled_features(_renderer);
    _bindlessSupported           = supportedFeatures & GERIUM_FEATURE_BINDLESS_BIT;
    _drawIndirectSupported       = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_BIT;
    _drawIndirectCountSupported  = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT;
    _8and16BitStorageSupported   = (supportedFeatures & GERIUM_FEATURE_8_BIT_STORAGE_BIT) &&
                                 (supportedFeatures & GERIUM_FEATURE_16_BIT_STORAGE_BIT);
    _isModernPipeline = _bindlessSupported && _drawIndirectSupported && _drawIndirectCountSupported;

    // Create the frame graph for managing rendering passes.
    check(gerium_frame_graph_create(_renderer, &_frameGraph));

    // Initialize the resource manager for managing textures, buffers, and other resources.
    _resourceManager.create(_renderer, _frameGraph);

    // Add rendering passes to the frame graph:
    // - PresentPass: Draws to the swapchain framebuffer and applies post-effects.
    // - GBufferPass: Generates the G-Buffer for opaque geometry.
    // - CullingPass: Performs GPU frustum culling (if modern pipeline is supported).
    // - MergeInstancesPass: Merges instances for CPU-side culling (fallback).
    addPass<PresentPass>();
    addPass<GBufferPass>();
    if (_isModernPipeline) {
        addPass<CullingPass>();
    } else {
        addPass<MergeInstancesPass>();
    }

    // Load and compile the frame graph for rendering.
    _resourceManager.loadFrameGraph(GRAPH_MAIN_ID);
    _resourceManager.loadFrameGraph(_isModernPipeline ? GRAPH_MODERN_ID : GRAPH_COMPAT_ID);
    check(gerium_frame_graph_compile(_frameGraph));

    // Initialize all rendering passes.
    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }

    // Set up multi-buffering for instances and materials.
    _maxFramesInFlight = gerium_renderer_get_frames_in_flight(_renderer);
    _instances.resize(_maxFramesInFlight);
    _materials.resize(_maxFramesInFlight);

    // Create an empty texture as a placeholder for unset textures.
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

    // If bindless textures are supported, initialize the bindless texture array.
    if (_bindlessSupported) {
        _bindlessTextures = _resourceManager.createDescriptorSet("", true);
        for (int i = 0; i < options.descriptor_pool_elements; ++i) {
            // Bind the empty texture to all slots in the bindless texture array.
            gerium_renderer_bind_texture(_renderer, _bindlessTextures, BINDLESS_BINDING, i, _emptyTexture);
        }
    }

    // Create dynamic buffers for draw data and scene data.
    _drawData =
        _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "draw_data", nullptr, sizeof(DrawData), 0);
    _activeScene = _resourceManager.createBuffer(
        GERIUM_BUFFER_USAGE_UNIFORM_BIT, true, "active_scene", nullptr, sizeof(SceneData));

    // Create a descriptor set for instance data and bind the draw data and command buffers.
    _instancesData = _resourceManager.createDescriptorSet("instances_data", true);
    gerium_renderer_bind_buffer(_renderer, _instancesData, 0, _drawData);
    gerium_renderer_bind_resource(_renderer, _instancesData, 3, "command_counts", false);
    gerium_renderer_bind_resource(_renderer, _instancesData, 4, "commands", false);

    // Create a descriptor set for scene data and bind the active scene buffer.
    _activeSceneData = _resourceManager.createDescriptorSet("scene_data", true);
    gerium_renderer_bind_buffer(_renderer, _activeSceneData, 0, _activeScene);

    // Set up reactive storage for handling entity creation, updates, and destruction.
    entityRegistry().storage<entt::reactive>(STORAGE_CONSTRUCT).on_construct<Renderable>().on_update<Renderable>();
    entityRegistry().storage<entt::reactive>(STORAGE_DESTROY).on_destroy<Renderable>().on_destroy<Static>();
    entityRegistry()
        .storage<entt::reactive>(STORAGE_UPDATE)
        .on_construct<Static>()
        .on_construct<Transform>()
        .on_update<Transform>()
        .on_destroy<Transform>();
}

// Cleans up resources and shuts down the RenderService.
void RenderService::stop() {
    // Disconnect reactive storage handlers.
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
        // Uninitialize all rendering passes.
        for (auto& renderPass : std::ranges::reverse_view(_renderPasses)) {
            renderPass->uninitialize(_frameGraph, _renderer);
        }

        // Clear compatibility data for legacy systems.
        _compatInstances = {};
        _compatMaterials = {};
        _compatMeshes    = {};
        _compatSceneData = {};

        // Clear materials, techniques, and textures.
        _textures               = {};
        _materialsNonCompressed = {};
        _materialsCompressed    = {};
        _materialsTable         = {};
        _techniquesTable        = {};
        _techniques             = {};

        // Clear instance data.
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

        // Clear scene data.
        _activeSceneData = {};
        _activeScene     = {};

        // Reset counters.
        _modelsDestroyed     = 0;
        _renderableDestroyed = 0;

        // Clear cluster data.
        _modelsInCluster.clear();
        _cluster = {};

        // Clear bindless textures and the empty texture.
        _bindlessTextures = {};
        _emptyTexture     = {};

        // Destroy all GPU resources managed by the resource manager.
        _resourceManager.destroy();

        // Destroy the frame graph.
        if (_frameGraph) {
            gerium_frame_graph_destroy(_frameGraph);
            _frameGraph = nullptr;
        }

        // Destroy the profiler.
        if (_profiler) {
            gerium_profiler_destroy(_profiler);
            _profiler = nullptr;
        }

        // Destroy the renderer.
        gerium_renderer_destroy(_renderer);
        _renderer = nullptr;

        // Clear any stored errors.
        _error = nullptr;
    }
}

// Updates the RenderService each frame, handling rendering and resource updates.
void RenderService::update(gerium_uint64_t elapsedMs, gerium_float64_t /* elapsed */) {
    // Start a new frame. If the renderer returns GERIUM_RESULT_SKIP_FRAME,
    // rendering is not possible (e.g., the window is minimized). In such cases,
    // the service can still perform logic updates (e.g., for networking).
    if (gerium_renderer_new_frame(_renderer) == GERIUM_RESULT_SKIP_FRAME) {
        return;
    }

    // Get the current size of the render area
    gerium_application_get_size(application().handle(), &_width, &_height);

    // Increment the frame number and update the frame index for multi-buffering.
    _frameIndex = gerium_uint32_t(_frame++ % _maxFramesInFlight);

    // Remove unused GPU resources to free up memory.
    _resourceManager.update(elapsedMs);

    // Update GPU resources as needed:
    updateCluster();          // Update the cluster data if geometry or statics have changed.
    updateActiveSceneData();  // Update scene data (e.g., camera matrices).
    updateDynamicInstances(); // Update dynamic instances (e.g., moving objects).
    updateInstancesData();    // Bind instance data to the descriptor set for rendering.

    // Execute the frame graph to render the current frame.
    gerium_renderer_render(_renderer, _frameGraph);
    gerium_renderer_present(_renderer);

    // If an unhandled exception occurred, rethrow it to terminate the service.
    if (_error) {
        std::rethrow_exception(_error);
    }
}

// Reloads the cluster by loading models and creating GPU buffers.
void RenderService::reloadCluster() {
    _modelsInCluster.clear(); // Clear the current model indices.

    Cluster cluster;
    std::map<entt::hashed_string, Model> models;

    // Helper function to load a model and add its meshes to the cluster.
    auto getModel = [this, &cluster, &models](const entt::hashed_string& modelId) -> const Model& {
        if (auto it = models.find(modelId); it != models.end()) {
            return it->second;
        }

        // Load the model if it hasn't been loaded yet.
        models[modelId] = loadModel(cluster, modelId);
        auto& model     = models[modelId];
        auto& indices   = _modelsInCluster[modelId].indices;

        // Store the node and mesh indices for the model.
        for (const auto& mesh : model.meshes) {
            indices.emplace_back(mesh.nodeIndex, mesh.meshIndex);
        }
        return model;
    };

    // Iterate over all Renderable entities and update their mesh indices.
    auto& registry = entityRegistry();
    for (auto entity : registry.view<Renderable>()) {
        auto& renderable = registry.get<Renderable>(entity);
        for (auto& mesh : renderable.meshes) {
            const auto& model = getModel(mesh.model); // Load the model if necessary.
            for (const auto& reloadMesh : model.meshes) {
                if (reloadMesh.nodeIndex == mesh.node) {
                    // Update the mesh index in the Renderable component.
                    mesh.mesh = reloadMesh.meshIndex;
                }
            }
        }
    }

    // If the cluster contains data, create GPU buffers for it.
    if (!cluster.vertices.empty()) {
        createCluster(cluster);
    }
}

// Creates a GPU cluster from the provided geometry data, filling SSBO buffers with vertex, index, and mesh data.
void RenderService::createCluster(const Cluster& cluster) {
    // Clear references to old GPU cluster data.
    _cluster = {};
    _resourceManager.update(0); // Free up unused resources.

    // If indirect draw count is not supported, store mesh data on the CPU for fallback rendering.
    if (!_drawIndirectCountSupported) {
        _compatMeshes = cluster.meshes;
    }

    // Fill the GPU buffer with vertex data.
    {
        gerium_cdata_t data;
        gerium_uint32_t size;
        std::vector<Vertex> vertices;

        // If 8-bit and 16-bit storage is supported, compress vertex data for efficiency.
        if (_8and16BitStorageSupported) {
            vertices.resize(cluster.vertices.size());
            for (size_t i = 0; i < cluster.vertices.size(); ++i) {
                const auto& v = cluster.vertices[i];
                const auto n  = glm::vec3(v.nx, v.ny, v.nz) * 127.0f + 127.5f; // Normalize and quantize normals.
                const auto t  = glm::vec3(v.tx, v.ty, v.tz) * 127.0f + 127.5f; // Normalize and quantize tangents.
                const auto s  = v.ts < 0.0f ? -1 : 1;                          // Determine tangent sign.

                vertices[i].px = meshopt_quantizeHalf(v.px); // Quantize vertex position.
                vertices[i].py = meshopt_quantizeHalf(v.py);
                vertices[i].pz = meshopt_quantizeHalf(v.pz);
                vertices[i].nx = uint8_t(n.x); // Store quantized normal.
                vertices[i].ny = uint8_t(n.y);
                vertices[i].nz = uint8_t(n.z);
                vertices[i].tx = uint8_t(t.x); // Store quantized tangent.
                vertices[i].ty = uint8_t(t.y);
                vertices[i].tz = uint8_t(t.z);
                vertices[i].ts = int8_t(s);                  // Store tangent sign.
                vertices[i].tu = meshopt_quantizeHalf(v.tu); // Quantize texture coordinates.
                vertices[i].tv = meshopt_quantizeHalf(v.tv);
            }
            data = (gerium_cdata_t) vertices.data();
            size = (gerium_uint32_t) (vertices.size() * sizeof(vertices[0]));
        } else {
            // Use uncompressed vertex data if compression is not supported.
            data = (gerium_cdata_t) cluster.vertices.data();
            size = (gerium_uint32_t) (cluster.vertices.size() * sizeof(cluster.vertices[0]));
        }

        // Create a GPU buffer for vertex data.
        _cluster.vertices =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "cluster_vertices", data, size, 0);
    }

    // Fill the GPU buffer with index data.
    {
        _cluster.indices =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                          false,
                                          "cluster_indices",
                                          (gerium_cdata_t) cluster.primitiveIndices.data(),
                                          (gerium_uint32_t) (cluster.primitiveIndices.size() * sizeof(uint32_t)),
                                          0);
    }

    // Fill the GPU buffer with mesh data (e.g., vertex offsets, bounding boxes, LODs).
    {
        gerium_cdata_t data;
        gerium_uint32_t size;
        std::vector<Mesh> meshes;

        // If 8-bit and 16-bit storage is supported, compress mesh data for efficiency.
        if (_8and16BitStorageSupported) {
            meshes.resize(cluster.meshes.size());
            for (size_t i = 0; i < cluster.meshes.size(); ++i) {
                const auto& m = cluster.meshes[i];

                meshes[i].center[0]    = meshopt_quantizeHalf(m.center[0]); // Quantize bounding sphere center.
                meshes[i].center[1]    = meshopt_quantizeHalf(m.center[1]);
                meshes[i].center[2]    = meshopt_quantizeHalf(m.center[2]);
                meshes[i].radius       = meshopt_quantizeHalf(m.radius);     // Quantize bounding sphere radius.
                meshes[i].bboxMin[0]   = meshopt_quantizeHalf(m.bboxMin[0]); // Quantize AABB min bounds.
                meshes[i].bboxMin[1]   = meshopt_quantizeHalf(m.bboxMin[1]);
                meshes[i].bboxMin[2]   = meshopt_quantizeHalf(m.bboxMin[2]);
                meshes[i].bboxMax[0]   = meshopt_quantizeHalf(m.bboxMax[0]); // Quantize AABB max bounds.
                meshes[i].bboxMax[1]   = meshopt_quantizeHalf(m.bboxMax[1]);
                meshes[i].bboxMax[2]   = meshopt_quantizeHalf(m.bboxMax[2]);
                meshes[i].vertexOffset = m.vertexOffset;      // Store vertex offset.
                meshes[i].vertexCount  = m.vertexCount;       // Store vertex count.
                meshes[i].lodCount     = uint8_t(m.lodCount); // Store LOD count.
                for (int l = 0; l < std::size(m.lods); ++l) {
                    meshes[i].lods[l] = m.lods[l]; // Store LOD data.
                }
            }
            data = (gerium_cdata_t) meshes.data();
            size = (gerium_uint32_t) (meshes.size() * sizeof(meshes[0]));
        } else {
            // Use uncompressed mesh data if compression is not supported.
            data = (gerium_cdata_t) cluster.meshes.data();
            size = (gerium_uint32_t) (cluster.meshes.size() * sizeof(cluster.meshes[0]));
        }

        // Create a GPU buffer for mesh data.
        _cluster.meshes =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, "cluster_meshes", data, size, 0);
    }

    // Create a descriptor set for the cluster data and bind the vertex, index, and mesh buffers.
    _cluster.ds = _resourceManager.createDescriptorSet("cluster_ds", true);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 0, _cluster.vertices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 1, _cluster.indices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 2, _cluster.meshes);
}

// Fills GPU SSBO buffers with static instances and their materials.
// Static instances and materials do not change, so their allocation is done once at startup.
void RenderService::createStaticInstances() {
    // Clear previous instance and material buffers.
    for (auto& buffer : _instances) {
        buffer = {};
    }
    for (auto& buffer : _materials) {
        buffer = {};
    }

    // Reset instance and material counters.
    _staticInstancesCount  = 0;
    _dynamicInstancesCount = 0;
    _staticMaterialsCount  = 0;
    _dynamicMaterialsCount = 0;

    // Clear cached data for techniques, textures, and materials.
    _techniques.clear();
    _textures.clear();
    _techniquesTable.clear();
    _materialsTable.clear();
    _materialsCompressed.clear();
    _materialsNonCompressed.clear();
    _compatMaterials.clear();
    _compatInstances.clear();

    // Free up unused resources.
    _resourceManager.update(0);

    // Get static instances and index their materials.
    std::vector<MeshInstance> instances = getInstances(true);

    // Prepare material data for GPU upload.
    gerium_cdata_t materialData;
    gerium_uint32_t materialSize;
    if (_8and16BitStorageSupported) {
        _staticMaterialsCount = (gerium_uint32_t) _materialsCompressed.size();
        // Reserve space for dynamic materials.
        _materialsCompressed.resize(_materialsCompressed.size() + MAX_DYNAMIC_MATERIALS);
        materialData = _materialsCompressed.data();
        materialSize = gerium_uint32_t(_materialsCompressed.size() * sizeof(_materialsCompressed[0]));
    } else {
        _staticMaterialsCount = (gerium_uint32_t) _materialsNonCompressed.size();
        // Reserve space for dynamic materials.
        _materialsNonCompressed.resize(_materialsNonCompressed.size() + MAX_DYNAMIC_MATERIALS);
        materialData = _materialsNonCompressed.data();
        materialSize = gerium_uint32_t(_materialsNonCompressed.size() * sizeof(_materialsNonCompressed[0]));
    }

    // Save the number of static instances.
    _staticInstancesCount = (gerium_uint32_t) instances.size();

    // If indirect draw count is not supported, store instances on the CPU for fallback rendering.
    if (!_drawIndirectCountSupported) {
        _compatInstances = instances;
    }

    // Reserve space for dynamic instances in the instance buffer.
    instances.resize(instances.size() + MAX_DYNAMIC_INSTANCES);
    assert(instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);

    // Create GPU buffers for materials and instances, with space for both static and dynamic data.
    for (size_t i = 0; i < _materials.size(); ++i) {
        const auto name = "materials_" + std::to_string(i);
        _materials[i]   = _resourceManager.createBuffer(
            GERIUM_BUFFER_USAGE_STORAGE_BIT, false, { name.c_str(), name.length() }, materialData, materialSize, 0);
    }
    for (size_t i = 0; i < _instances.size(); ++i) {
        const auto name = "instances_" + std::to_string(i);
        _instances[i]   = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                      false,
                                                        { name.c_str(), name.length() },
                                                      (gerium_cdata_t) instances.data(),
                                                      (gerium_uint32_t) (instances.size() * sizeof(instances[0])),
                                                      0);
    }

    // Clear CPU-side material data after uploading to the GPU.
    _materialsCompressed.clear();
    _materialsNonCompressed.clear();
}

// This method checks whether the entire cluster needs to be updated (and
// starts recreating it if necessary). It also checks whether static data
// needs to be updated (and recreates it if necessary).
//
// Basically, the cluster and statics will only update in developer mode
// when creating a map, during gameplay, the statics will not change
void RenderService::updateCluster() {
    // By default, assume that neither the cluster nor the static instances need to be updated.
    auto needReloadCluster         = false;
    auto needUpdateStaticInstances = false;

    // Check for new, destroyed, and updated components.
    checkNewComponents(needReloadCluster, needUpdateStaticInstances);
    checkDestroyedComponents(needReloadCluster, needUpdateStaticInstances);
    checkUpdatedComponents(needReloadCluster, needUpdateStaticInstances);

    // Reload the cluster if necessary.
    if (needReloadCluster) {
        reloadCluster();
    }

    // Update static instances if necessary.
    if (needUpdateStaticInstances) {
        createStaticInstances();
    }
}

// Updates the _activeScene UBO buffer with camera data.
void RenderService::updateActiveSceneData() {
    auto view = entityRegistry().view<Camera>();

    // Find the active camera in the registry.
    Camera* camera = nullptr;
    for (auto entity : view) {
        if (auto& c = view.get<Camera>(entity); c.active) {
            camera = &c;
        }
    }
    assert(camera); // Ensure an active camera exists.

    // Calculate frustum planes and other camera-related data.
    auto projectionT = glm::transpose(camera->projection);

    vec4 frustumX = projectionT[3] + projectionT[0];
    vec4 frustumY = projectionT[3] + projectionT[1];
    frustumX /= glm::length(frustumX.xyz());
    frustumY /= glm::length(frustumY.xyz());

    auto invResolution = glm::vec2{ 1.0f / camera->resolution.x, 1.0f / camera->resolution.y };

    auto p00p11 = glm::vec2(camera->projection[0][0], camera->projection[1][1]);

    // Populate the SceneData structure with camera data.
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

    // Map the GPU buffer and copy the scene data to it.
    auto data = (SceneData*) gerium_renderer_map_buffer(_renderer, _activeScene, 0, sizeof(SceneData));
    memcpy(data, &sceneData, sizeof(SceneData));
    gerium_renderer_unmap_buffer(_renderer, _activeScene);

    // If indirect draw count is not supported, store the scene data on the CPU for fallback rendering.
    if (!_drawIndirectCountSupported) {
        _compatSceneData = sceneData;
    }
}

// Updates dynamic instances, fills dynamic GPU buffers (optimal for writing every frame
// from the CPU side) with materials and instances.
void RenderService::updateDynamicInstances() {
    // Pointer and size for material data to be uploaded to the GPU.
    gerium_cdata_t materialData;
    gerium_uint32_t materialSize;

    // Retrieve dynamic instances (e.g., moving objects) from the scene.
    std::vector<MeshInstance> instances = getInstances(false); // `false` indicates dynamic instances.

    // Ensure the total number of instances (static + dynamic) does not exceed the maximum allowed.
    assert(_staticInstancesCount + instances.size() < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);

    // Save the number of dynamic instances for later use.
    _dynamicInstancesCount = (gerium_uint32_t) instances.size();

    // Determine which material buffer to use based on GPU feature support.
    if (_8and16BitStorageSupported) {
        // Use compressed materials if 8-bit and 16-bit storage is supported.
        _dynamicMaterialsCount = (gerium_uint32_t) _materialsCompressed.size();
        materialData           = _materialsCompressed.data();
        materialSize           = gerium_uint32_t(_materialsCompressed.size() * sizeof(Material));
    } else {
        // Use uncompressed materials as a fallback.
        _dynamicMaterialsCount = (gerium_uint32_t) _materialsNonCompressed.size();
        materialData           = _materialsNonCompressed.data();
        materialSize           = gerium_uint32_t(_materialsNonCompressed.size() * sizeof(MaterialNonCompressed));
    }

    // If there are dynamic materials, create a GPU buffer for them.
    if (_dynamicMaterialsCount) {
        _dynamicMaterials =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "", materialData, materialSize, 0);
    }

    // If there are dynamic instances, create a GPU buffer for them.
    if (_dynamicInstancesCount) {
        _dynamicInstances = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                          true,
                                                          "",
                                                          (gerium_cdata_t) instances.data(),
                                                          (gerium_uint32_t) (instances.size() * sizeof(instances[0])),
                                                          0);
    }

    // If indirect draw count is not supported, store dynamic instances on the CPU side for fallback rendering.
    if (!_drawIndirectCountSupported) {
        _compatInstances.resize(_staticInstancesCount + _dynamicInstancesCount);
        for (gerium_uint32_t i = 0; i < _dynamicInstancesCount; ++i) {
            _compatInstances[_staticInstancesCount + i] = instances[i];
        }
    }

    // Note: We do not clear _materialsCompressed and _materialsNonCompressed here because they are needed
    // to update dynamic materials every frame. New dynamic materials will be appended to these buffers.
}

// Updates data for the camera and re-binds the SSBO for materials and instances to the descriptor set.
void RenderService::updateInstancesData() {
    auto drawData       = (DrawData*) gerium_renderer_map_buffer(_renderer, _drawData, 0, sizeof(DrawData));
    drawData->drawCount = instancesCount();
    gerium_renderer_unmap_buffer(_renderer, _drawData);

    // Re-bind the materials and instances buffers to the descriptor set for rendering.
    gerium_renderer_bind_buffer(_renderer, _instancesData, 1, _materials[_frameIndex]);
    gerium_renderer_bind_buffer(_renderer, _instancesData, 2, _instances[_frameIndex]);
}

// Check for update cluster or static instances
void RenderService::checkNewComponents(bool& needReloadCluster, bool& needUpdateStaticInstances) {
    auto& registry = entityRegistry();
    auto& storage  = registry.storage<entt::reactive>(STORAGE_CONSTRUCT);

    // Check for new or updated Renderable components in the reactive storage.
    for (auto entity : storage) {
        auto& renderable    = registry.get<Renderable>(entity);
        const auto isStatic = registry.any_of<Static>(entity);

        // If the entity has a Static component, static instances need to be updated.
        if (isStatic) {
            needUpdateStaticInstances = true;
        }

        // Check if the meshes referenced by the Renderable are in the current GPU cluster.
        for (auto& mesh : renderable.meshes) {
            // If the mesh is in the cluster, update its index in the Renderable component.
            if (auto it = _modelsInCluster.find(mesh.model); it != _modelsInCluster.end()) {
                for (const auto& index : it->second.indices) {
                    if (mesh.node == index.nodeIndex) {
                        mesh.mesh = index.meshIndex;
                        break;
                    }
                }
            } else {
                // If the mesh is not in the cluster, the entire cluster and static instances need to be updated.
                needReloadCluster         = true;
                needUpdateStaticInstances = true;
                break;
            }
        }

        // If the cluster needs to be reloaded, no further checks are needed.
        if (needReloadCluster) {
            break;
        }
    }
    storage.clear(); // Clear the create storage after processing.
}

// Check for update cluster or static instances
void RenderService::checkDestroyedComponents(bool& needReloadCluster, bool& needUpdateStaticInstances) {
    auto& registry = entityRegistry();
    auto& storage  = registry.storage<entt::reactive>(STORAGE_DESTROY);

    // If static instances don't need to be updated yet, check for destroyed Renderable or Static components.
    if (!needUpdateStaticInstances && !storage.empty()) {
        // Count the current number of static instances in the registry.
        gerium_uint32_t currentStaticInstancesCount = 0;
        for (auto _ : registry.view<Renderable, Static>()) {
            ++currentStaticInstancesCount;
        }

        // If the number of static instances has changed, update the static instances.
        needUpdateStaticInstances = currentStaticInstancesCount != _staticInstancesCount;
    }

    // Track the number of destroyed Renderable components.
    _renderableDestroyed += (gerium_uint32_t) storage.size();

    // If the number of destroyed components exceeds a threshold, check for unused models in the cluster.
    if (_renderableDestroyed >= deletionsToCheckUpdateCluster) {
        std::set<entt::hashed_string> removedModels;
        for (const auto& [key, _] : _modelsInCluster) {
            removedModels.insert(key);
        }

        // Save only removed models in removedModels.
        for (auto [entity, renderable] : registry.view<Renderable>().each()) {
            for (const auto& mesh : renderable.meshes) {
                if (auto it = removedModels.find(mesh.model); it != removedModels.end()) {
                    removedModels.erase(it);
                }
            }
        }

        // Track the number of removed models.
        _modelsDestroyed += (gerium_uint32_t) removedModels.size();
        _renderableDestroyed = 0; // Reset the counter.
    }
    storage.clear(); // Clear the destroy storage after processing.

    // If the number of removed models exceeds a threshold, reload the cluster and update static instances.
    if (_modelsDestroyed >= deletionsToCheckUpdateCluster) {
        needReloadCluster         = true;
        needUpdateStaticInstances = true;
        _modelsDestroyed          = 0; // Reset the counter.
    }
}

// Check for update cluster or static instances
void RenderService::checkUpdatedComponents(bool& /* needReloadCluster */, bool& needUpdateStaticInstances) {
    auto& registry = entityRegistry();
    auto& storage  = registry.storage<entt::reactive>(STORAGE_UPDATE);

    // Check for updates to entities with Static or Transform components.
    if (!needUpdateStaticInstances && !storage.empty()) {
        for (auto entity : storage) {
            if (entityRegistry().any_of<Static>(entity)) {
                needUpdateStaticInstances = true;
                break;
            }
        }
    }
    storage.clear(); // Clear the update storage after processing.
}

// Returns a list of static or dynamic instances (meshes with transforms) from the scene.
std::vector<MeshInstance> RenderService::getInstances(bool isStatic) {
    std::vector<MeshInstance> instances;

    // Helper lambda to iterate over entities and add their instances to the list.
    auto emplace = [this, &instances](auto&& view) {
        for (auto entity : view) {
            auto& renderable = view.get<Renderable>(entity);
            auto& transform  = view.get<Transform>(entity);
            emplaceInstance(renderable, transform, instances);
        }
    };

    // Depending on whether we need static or dynamic instances, use the appropriate view.
    if (isStatic) {
        // Get all entities with Renderable, Transform, and Static components.
        emplace(entityRegistry().view<Renderable, Transform, Static>());
    } else {
        // Get all entities with Renderable and Transform components, excluding those with Static.
        emplace(entityRegistry().view<Renderable, Transform>(entt::exclude<Static>));
    }
    return instances;
}

// Adds an instance (mesh with transform) to the list of instances.
void RenderService::emplaceInstance(const Renderable& renderable,
                                    const Transform& transform,
                                    std::vector<MeshInstance>& instances) {
    // Iterate over all meshes in the Renderable component.
    for (const auto& meshData : renderable.meshes) {
        instances.push_back({}); // Add a new instance to the list.
        auto& instance = instances.back();
        auto scale     = glm::max(glm::max(transform.scale.x, transform.scale.y), transform.scale.z);

        // Fill the instance data:
        instance.world        = transform.matrix;     // World transformation matrix.
        instance.prevWorld    = transform.prevMatrix; // Previous frame's world matrix (for motion vectors).
        instance.normalMatrix = glm::transpose(glm::inverse(transform.matrix)); // Normal matrix for lighting.
        instance.scale        = scale;                                          // Scale factor.
        instance.mesh         = meshData.mesh;                                  // Index of the mesh in the cluster.
        instance.technique    = getOrEmplaceTechnique(meshData.material);       // Index of the rendering technique.
        instance.material     = getOrEmplaceMaterial(meshData.material);        // Index of the material.
    }
}

// Loads and indexes a rendering technique. If the technique is already loaded, returns its index.
gerium_uint32_t RenderService::getOrEmplaceTechnique(const MaterialData& material) {
    gerium_uint32_t techniqueIndex;

    // Check if the technique is already in the cache.
    if (auto it = _techniquesTable.find(material.name); it != _techniquesTable.end()) {
        techniqueIndex = it->second; // Use the cached index.
    } else {
        // Load the technique from the resource manager.
        auto technique = _resourceManager.loadTechnique(material.name);

        // Add the technique to the list and cache its index.
        techniqueIndex = (gerium_uint32_t) _techniques.size();
        _techniques.push_back(technique);
        _techniquesTable.insert({ material.name, techniqueIndex });

        // Ensure the number of techniques does not exceed the maximum allowed.
        assert(_techniques.size() < MAX_TECHNIQUES);
    }
    return techniqueIndex;
}

// Loads and indexes a material. If the material is already loaded, returns its index.
gerium_uint32_t RenderService::getOrEmplaceMaterial(const MaterialData& material) {
    // Compute a hash for the material data to enable efficient lookups.
    const auto hash = materialDataHash(material);

    gerium_uint32_t materialIndex = 0;

    // Check if the material is already in the cache.
    if (auto it = _materialsTable.find(hash); it != _materialsTable.end()) {
        materialIndex = it->second; // Use the cached index.
    } else {
        // Add the material to the cache and assign it a new index.
        materialIndex = (gerium_uint32_t) _materialsTable.size();
        _materialsTable.insert({ hash, materialIndex });

        // Helper lambda to load a texture and bind it if bindless textures are supported.
        auto loadTexture = [this](const entt::hashed_string& name) -> gerium_uint16_t {
            if (name.size()) {
                // Load the texture from the resource manager.
                Texture result = _resourceManager.loadTexture(name);
                _textures.insert(result);
                gerium_texture_h texture = result;

                // If bindless textures are supported, bind the texture to the bindless array.
                if (_bindlessSupported) {
                    gerium_renderer_bind_texture(
                        _renderer, _bindlessTextures, BINDLESS_BINDING, texture.index, texture);
                }
                return texture.index; // Return the texture index.
            }

            // If the texture name is empty, use the empty texture as a fallback.
            gerium_texture_h emptyTexture = _emptyTexture;
            return emptyTexture.index;
        };

        // Helper lambda to create and populate a material structure.
        auto emplaceMaterial = [this, &loadTexture](auto& materials, const MaterialData& material, auto pred) {
            materials.push_back({}); // Add a new material to the list.
            auto& mat = materials.back();

            // Fill the material data:
            mat.baseColorFactor[0]       = pred(material.baseColorFactor.x); // Base color (R).
            mat.baseColorFactor[1]       = pred(material.baseColorFactor.y); // Base color (G).
            mat.baseColorFactor[2]       = pred(material.baseColorFactor.z); // Base color (B).
            mat.baseColorFactor[3]       = pred(material.baseColorFactor.w); // Base color (A).
            mat.emissiveFactor[0]        = pred(material.emissiveFactor.x);  // Emissive color (R).
            mat.emissiveFactor[1]        = pred(material.emissiveFactor.y);  // Emissive color (G).
            mat.emissiveFactor[2]        = pred(material.emissiveFactor.z);  // Emissive color (B).
            mat.metallicFactor           = pred(material.metallicFactor);    // Metallic factor.
            mat.roughnessFactor          = pred(material.roughnessFactor);   // Roughness factor.
            mat.occlusionStrength        = pred(material.occlusionStrength); // Occlusion strength.
            mat.alphaCutoff              = pred(material.alphaCutoff);       // Alpha cutoff.
            mat.baseColorTexture         = loadTexture(material.baseColorTexture);
            mat.metallicRoughnessTexture = loadTexture(material.metallicRoughnessTexture);
            mat.normalTexture            = loadTexture(material.normalTexture);
            mat.occlusionTexture         = loadTexture(material.occlusionTexture);
            mat.emissiveTexture          = loadTexture(material.emissiveTexture);
            return mat;
        };

        // Add the material to the appropriate buffer (compressed or uncompressed).
        if (_8and16BitStorageSupported) {
            emplaceMaterial(_materialsCompressed, material, meshopt_quantizeHalf); // Use compressed format.
        } else {
            emplaceMaterial(_materialsNonCompressed, material, [](const auto value) {
                return value; // Use uncompressed format.
            });
        }

        // If bindless textures are not supported, store the material on the CPU for fallback rendering.
        if (!_bindlessSupported) {
            emplaceMaterial(_compatMaterials, material, [](const auto value) {
                return value; // Use uncompressed format.
            });
        }
    }
    return materialIndex;
}

// Computes a hash for the material data to enable efficient lookups and caching.
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

// The C callback function to wrap the C++ `prepare` method of a RenderPass.
gerium_uint32_t RenderService::prepare(gerium_frame_graph_t frameGraph,
                                       gerium_renderer_t renderer,
                                       gerium_uint32_t maxWorkers,
                                       gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        return renderPass->prepare(frameGraph, renderer, maxWorkers);
    } catch (...) {
        // If an exception occurs, store it in the RenderService and return 0.
        renderPass->renderService()._error = std::current_exception();
        return 0;
    }
}

// The C callback function to wrap the C++ `resize` method of a RenderPass.
gerium_bool_t RenderService::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data) {
    auto renderPass = (RenderPass*) data;
    try {
        renderPass->resize(frameGraph, renderer);
    } catch (...) {
        // If an exception occurs, store it in the RenderService and return false.
        renderPass->renderService()._error = std::current_exception();
        return false;
    }
    return true;
}

// The C callback function to wrap the C++ `render` method of a RenderPass.
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
        // If an exception occurs, store it in the RenderService and return false.
        renderPass->renderService()._error = std::current_exception();
        return false;
    }
    return true;
}
