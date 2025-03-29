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
    if (_instances.dynamicMaterialsCount) {
        // If there are dynamic materials, copy them to the end of the static materials buffer.
        // The size of each material depends on whether 8-bit and 16-bit storage is supported.
        const auto size = _features._8and16BitStorage ? sizeof(Material) : sizeof(MaterialNonCompressed);
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _instances.dynamicMaterials,
                                          0,
                                          _instances.materials[_frame.frameIndex],
                                          _instances.staticMaterialsCount * size,
                                          _instances.dynamicMaterialsCount * size);
    }
    if (_instances.dynamicInstancesCount) {
        // If there are dynamic instances, copy them to the end of the static instances buffer.
        gerium_command_buffer_copy_buffer(commandBuffer,
                                          _instances.dynamicInstances,
                                          0,
                                          _instances.instances[_frame.frameIndex],
                                          _instances.staticInstancesCount * sizeof(MeshInstance),
                                          _instances.dynamicInstancesCount * sizeof(MeshInstance));
    }
}

// Marks statics as changed
void RenderService::onDirtyScene(const DirtySceneEvent& event) {
    if (event.hasStatics) {
        _instances.isDirtyStatics = true;
    }
}

// Initializes the RenderService, setting up the renderer, frame graph, and resources.
void RenderService::start() {
    const auto& settings = entityRegistry().ctx().get<Settings>();

    // We will explicitly set some parameters necessary, which are sufficient
    // for this example. If the option is not set, the default value will be used
    gerium_renderer_options_t options{};
    options.debug_mode                = settings.debugMode; // Validation layers, GPU object names, and logs
    options.app_version               = settings.version;
    options.command_buffers_per_frame = 5;
    options.descriptor_sets_pool_size = 128;
    options.descriptor_pool_elements  = 128;
    options.dynamic_ssbo_size         = 64 * 1024 * 1024;

    initRenderer(options);
    initFrameGraph();
    initTextures(options.descriptor_pool_elements);
    initData();
    initCluster();

    // Subscribe to scene change events
    application().dispatcher().sink<DirtySceneEvent>().connect<&RenderService::onDirtyScene>(*this);
}

// Cleans up resources and shuts down the RenderService.
void RenderService::stop() {
    application().dispatcher().sink<DirtySceneEvent>().disconnect(this);

    if (_renderer) {
        // Uninitialize all rendering passes.
        for (auto& renderPass : std::ranges::reverse_view(_renderPasses)) {
            renderPass->uninitialize(_frameGraph, _renderer);
        }

        _compat           = {}; // Clear compatibility data for legacy systems.
        _instances        = {}; // Clear instances data.
        _scene            = {}; // Clear scene data.
        _cluster          = {}; // Clear cluster data.
        _frame            = {}; // Clear frame data.
        _bindlessTextures = {}; // Clear bindless textures.
        _emptyTexture     = {}; // Clear empty texture.
        _features         = {}; // Clear supported features.

        // Clear cache render passes
        _renderPassesCache.clear();
        _renderPasses.clear();

        // Destroy all GPU resources managed by the resource manager.
        _resourceManager.destroy();

        // Destroy the frame graph.
        if (_frameGraph) {
            gerium_frame_graph_destroy(_frameGraph);
            _frameGraph = nullptr;
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
    gerium_application_get_size(application().handle(), &_frame.width, &_frame.height);

    // Increment the frame number and update the frame index for multi-buffering.
    _frame.frameIndex = gerium_uint32_t(_frame.frame++ % _frame.maxFramesInFlight);

    // Remove unused GPU resources to free up memory.
    _resourceManager.update(elapsedMs);

    // Update GPU resources as needed:
    updateActiveSceneData();  // Update scene data (e.g., camera matrices).
    updateStaticInstances();  // Update static instances (e.g., non-moving objects).
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

// Create a renderer handle
void RenderService::initRenderer(const gerium_renderer_options_t& options) {
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

    // Check which requested features are supported by the GPU.
    const auto supportedFeatures = gerium_renderer_get_enabled_features(_renderer);
    _features.bindless           = supportedFeatures & GERIUM_FEATURE_BINDLESS_BIT;
    _features.drawIndirect       = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_BIT;
    _features.drawIndirectCount  = supportedFeatures & GERIUM_FEATURE_DRAW_INDIRECT_COUNT_BIT;
    _features._8and16BitStorage  = (supportedFeatures & GERIUM_FEATURE_8_BIT_STORAGE_BIT) &&
                                  (supportedFeatures & GERIUM_FEATURE_16_BIT_STORAGE_BIT);
    _features.isModernPipeline = _features.bindless && _features.drawIndirect && _features.drawIndirectCount;

    // Set up multi-buffering for instances and materials.
    _frame.maxFramesInFlight = gerium_renderer_get_frames_in_flight(_renderer);
}

// Add render passes, load and compile Frame Graph
void RenderService::initFrameGraph() {
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
    if (_features.isModernPipeline) {
        addPass<CullingPass>();
    } else {
        addPass<MergeInstancesPass>();
    }

    // Load and compile the frame graph for rendering.
    _resourceManager.loadFrameGraph(GRAPH_MAIN_ID);
    _resourceManager.loadFrameGraph(_features.isModernPipeline ? GRAPH_MODERN_ID : GRAPH_COMPAT_ID);
    check(gerium_frame_graph_compile(_frameGraph));

    // Initialize all rendering passes.
    for (auto& renderPass : _renderPasses) {
        renderPass->initialize(_frameGraph, _renderer);
    }
}

// Create an empty texture as a placeholder for unset textures and bindless textures.
void RenderService::initTextures(gerium_uint32_t elementCount) {
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
    if (_features.bindless) {
        _bindlessTextures = _resourceManager.createDescriptorSet("", true);
        for (int i = 0; i < elementCount; ++i) {
            // Bind the empty texture to all slots in the bindless texture array.
            gerium_renderer_bind_texture(_renderer, _bindlessTextures, BINDLESS_BINDING, i, _emptyTexture);
        }
    }
}

// Create GPU buffers and Descriptor Sets
void RenderService::initData() {
    constexpr auto flags = GERIUM_BUFFER_USAGE_UNIFORM_BIT;

    // Create a descriptor set for scene data and bind the active scene buffer.
    _scene.ds     = _resourceManager.createDescriptorSet("scene_data", true);
    _scene.buffer = _resourceManager.createBuffer(flags, true, "active_scene", nullptr, sizeof(SceneData));
    gerium_renderer_bind_buffer(_renderer, _scene.ds, 0, _scene.buffer);

    // Create a descriptor set for instance data and bind the draw data and command buffers.
    _instances.ds       = _resourceManager.createDescriptorSet("instances_data", true);
    _instances.drawData = _resourceManager.createBuffer(flags, true, "draw_data", nullptr, sizeof(DrawData), 0);
    gerium_renderer_bind_buffer(_renderer, _instances.ds, 0, _instances.drawData);
    gerium_renderer_bind_resource(_renderer, _instances.ds, 3, "command_counts", false);
    gerium_renderer_bind_resource(_renderer, _instances.ds, 4, "commands", false);

    // Resize buffers
    _instances.instances.resize(_frame.maxFramesInFlight);
    _instances.materials.resize(_frame.maxFramesInFlight);
}

// Load all geometry
void RenderService::initCluster() {
    Cluster cluster;
    for (const auto& modelId : modelIds) {
        const auto model = loadModel(cluster, modelId);
        auto& nodes      = _cluster.models[modelId].nodes;
        // Store the node and mesh indices for the model.
        for (const auto& mesh : model.meshes) {
            nodes.emplace_back(mesh.nodeIndex, mesh.meshIndex);
        }
    }

    // Skip empty clusters
    if (cluster.vertices.empty()) {
        return;
    }

    // If indirect draw count or bindless textures is not supported, store mesh data on
    // the CPU for fallback rendering.
    if (!_features.isModernPipeline) {
        _compat.meshes = cluster.meshes;
    }

    // Fill the GPU buffer with vertex data.
    {
        const auto vertices = prepareVertices(cluster);
        _cluster.vertices   = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                          false,
                                                          "cluster_vertices",
                                                          (gerium_cdata_t) vertices.data(),
                                                          (gerium_uint32_t) vertices.size(),
                                                          0);
    }

    // Fill the GPU buffer with index data.
    {
        const auto indices = prepareIndices(cluster);
        _cluster.indices   = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                         false,
                                                         "cluster_indices",
                                                         (gerium_cdata_t) indices.data(),
                                                         (gerium_uint32_t) indices.size(),
                                                         0);
    }

    // Fill the GPU buffer with mesh data (e.g., vertex offsets, bounding boxes, LODs).
    {
        const auto meshes = prepareMeshes(cluster);
        _cluster.meshes   = _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT,
                                                        false,
                                                        "cluster_meshes",
                                                        (gerium_cdata_t) meshes.data(),
                                                        (gerium_uint32_t) meshes.size(),
                                                        0);
    }

    // Create a descriptor set for the cluster data and bind the vertex, index, and mesh buffers.
    _cluster.ds = _resourceManager.createDescriptorSet("cluster_ds", true);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 0, _cluster.vertices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 1, _cluster.indices);
    gerium_renderer_bind_buffer(_renderer, _cluster.ds, 2, _cluster.meshes);
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
    auto data = (SceneData*) gerium_renderer_map_buffer(_renderer, _scene.buffer, 0, sizeof(SceneData));
    memcpy(data, &sceneData, sizeof(SceneData));
    gerium_renderer_unmap_buffer(_renderer, _scene.buffer);

    // If indirect draw count or bindless textures is not supported, store the scene data
    // on the CPU for fallback rendering.
    if (!_features.isModernPipeline) {
        _compat.sceneData = sceneData;
    }
}

// Fills GPU SSBO buffers with static instances and their materials.
// Static instances and materials do not change, so their allocation is done once at startup.
void RenderService::updateStaticInstances() {
    if (!_instances.isDirtyStatics) {
        return;
    }
    _instances.isDirtyStatics = false;

    // Clear previous instance and material buffers.
    for (auto& buffer : _instances.instances) {
        buffer = {};
    }
    for (auto& buffer : _instances.materials) {
        buffer = {};
    }

    // Clear cached data for techniques, textures, and materials.
    _instances.techniques.clear();
    _instances.techniquesTable.clear();
    _instances.materialsTable.clear();
    _instances.dynamicMaterialData.clear();
    _compat.materials.clear();
    _compat.instances.clear();

    // Free up unused resources.
    _resourceManager.update(0);
    _instances.textures.clear();

    // Get static instances and index their materials.
    std::vector<MeshInstance> instances;
    std::vector<MaterialNonCompressed> materials;
    getInstances(true, instances, materials);

    // If indirect draw count or bindless textures is not supported, store instances on
    // the CPU for fallback rendering.
    if (!_features.isModernPipeline) {
        _compat.instances = instances;
    }

    // If bindless textures are not supported, store the material on the CPU for fallback rendering.
    if (!_features.bindless) {
        _compat.materials = materials;
    }

    // Save the number of static instances.
    _instances.staticInstancesCount = (gerium_uint32_t) instances.size();
    _instances.staticMaterialsCount = (gerium_uint32_t) materials.size();

    std::vector<Material> tmp;
    const auto maxMaterials = _instances.staticMaterialsCount + MAX_DYNAMIC_MATERIALS;
    const auto maxInstances = _instances.staticInstancesCount + MAX_DYNAMIC_INSTANCES;
    const auto mSpan        = prepareMaterials(materials, tmp, _instances.staticMaterialsCount, maxMaterials);
    const auto iSpan        = prepareInstances(instances, maxInstances);
    for (size_t i = 0; i < _instances.materials.size(); ++i) {
        const auto name = "materials_" + std::to_string(i);
        const auto hash = entt::hashed_string{ name.c_str(), name.length() };
        const auto data = (gerium_cdata_t) mSpan.data();
        const auto size = (gerium_uint32_t) mSpan.size();
        _instances.materials[i] =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, hash, data, size, 0);
    }
    for (size_t i = 0; i < _instances.instances.size(); ++i) {
        const auto name = "instances_" + std::to_string(i);
        const auto hash = entt::hashed_string{ name.c_str(), name.length() };
        const auto data = (gerium_cdata_t) iSpan.data();
        const auto size = (gerium_uint32_t) iSpan.size();
        _instances.instances[i] =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, false, hash, data, size, 0);
    }
}

// Updates dynamic instances, fills dynamic GPU buffers (optimal for writing every frame
// from the CPU side) with materials and instances.
void RenderService::updateDynamicInstances() {
    // Retrieve dynamic instances (e.g., moving objects) from the scene.
    std::vector<MeshInstance> instances;
    getInstances(false, instances, _instances.dynamicMaterialData); // `false` indicates dynamic instances.

    // Save the number of dynamic instances for later use.
    _instances.dynamicInstancesCount = (gerium_uint32_t) instances.size();
    _instances.dynamicMaterialsCount = (gerium_uint32_t) _instances.dynamicMaterialData.size();

    // If there are dynamic materials, create a GPU buffer for them.
    if (_instances.dynamicMaterialsCount) {
        std::vector<Material> tmp;
        const auto count = _instances.dynamicMaterialsCount;
        const auto span  = prepareMaterials(_instances.dynamicMaterialData, tmp, count, count);
        const auto data  = (gerium_cdata_t) span.data();
        const auto size  = (gerium_uint32_t) span.size();
        _instances.dynamicMaterials =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "", data, size, 0);
    }

    // If there are dynamic instances, create a GPU buffer for them.
    if (_instances.dynamicInstancesCount) {
        const auto span = prepareInstances(instances, _instances.dynamicInstancesCount);
        const auto data = (gerium_cdata_t) span.data();
        const auto size = (gerium_uint32_t) span.size();
        _instances.dynamicInstances =
            _resourceManager.createBuffer(GERIUM_BUFFER_USAGE_STORAGE_BIT, true, "", data, size, 0);
    }

    // If indirect draw count or bindless textures is not supported, store dynamic instances on
    // the CPU side for fallback rendering.
    if (!_features.isModernPipeline) {
        _compat.instances.resize(_instances.staticInstancesCount);
        _compat.instances.insert(_compat.instances.end(), instances.begin(), instances.end());
    }

    // If bindless textures are not supported, store the material on the CPU for fallback rendering.
    if (!_features.bindless) {
        _compat.materials.resize(_instances.staticMaterialsCount);
        _compat.materials.insert(
            _compat.materials.end(), _instances.dynamicMaterialData.begin(), _instances.dynamicMaterialData.end());
    }
}

// Updates data for the camera and re-binds the SSBO for materials and instances to the descriptor set.
void RenderService::updateInstancesData() {
    auto drawData       = (DrawData*) gerium_renderer_map_buffer(_renderer, _instances.drawData, 0, sizeof(DrawData));
    drawData->drawCount = instancesCount();
    gerium_renderer_unmap_buffer(_renderer, _instances.drawData);

    // Re-bind the materials and instances buffers to the descriptor set for rendering.
    gerium_renderer_bind_buffer(_renderer, _instances.ds, 1, _instances.materials[_frame.frameIndex]);
    gerium_renderer_bind_buffer(_renderer, _instances.ds, 2, _instances.instances[_frame.frameIndex]);
}

// Returns a list of static or dynamic instances (meshes with transforms) from the scene.
void RenderService::getInstances(bool isStatic,
                                 std::vector<MeshInstance>& instances,
                                 std::vector<MaterialNonCompressed>& materials) {
    // Helper lambda to iterate over entities and add their instances to the list.
    auto emplace = [this, &instances, &materials](auto&& view) {
        for (auto entity : view) {
            auto& renderable  = view.get<Renderable>(entity);
            auto& transform   = view.get<Transform>(entity);
            auto existsMeshes = false;
            for (auto& mesh : renderable.meshes) {
                if (auto it = _cluster.models.find(mesh.model); it != _cluster.models.end()) {
                    for (const auto& node : it->second.nodes) {
                        if (mesh.node == node.nodeIndex) {
                            mesh.mesh    = node.meshIndex; // reassign mesh index
                            existsMeshes = true;
                            break;
                        }
                    }
                }
            }
            if (existsMeshes) {
                emplaceInstance(renderable, transform, instances, materials);
            }
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
}

// Adds an instance (mesh with transform) to the list of instances.
void RenderService::emplaceInstance(const Renderable& renderable,
                                    const Transform& transform,
                                    std::vector<MeshInstance>& instances,
                                    std::vector<MaterialNonCompressed>& materials) {
    // Iterate over all meshes in the Renderable component.
    for (const auto& meshData : renderable.meshes) {
        instances.push_back({}); // Add a new instance to the list.
        auto& instance = instances.back();
        auto scale     = glm::max(glm::max(transform.scale.x, transform.scale.y), transform.scale.z);

        // Fill the instance data:
        instance.world        = transform.matrix;     // World transformation matrix.
        instance.prevWorld    = transform.prevMatrix; // Previous frame's world matrix (for motion vectors).
        instance.normalMatrix = glm::transpose(glm::inverse(transform.matrix));     // Normal matrix for lighting.
        instance.scale        = scale;                                              // Scale factor.
        instance.mesh         = meshData.mesh;                                      // Index of the mesh in the cluster.
        instance.technique    = getOrEmplaceTechnique(meshData.material);           // Index of the rendering technique.
        instance.material     = getOrEmplaceMaterial(meshData.material, materials); // Index of the material.
    }
}

// Loads and indexes a rendering technique. If the technique is already loaded, returns its index.
gerium_uint32_t RenderService::getOrEmplaceTechnique(const MaterialData& material) {
    gerium_uint32_t techniqueIndex;

    // Check if the technique is already in the cache.
    if (auto it = _instances.techniquesTable.find(material.name); it != _instances.techniquesTable.end()) {
        techniqueIndex = it->second; // Use the cached index.
    } else {
        // Load the technique from the resource manager.
        auto technique = _resourceManager.loadTechnique(material.name);

        // Add the technique to the list and cache its index.
        techniqueIndex = (gerium_uint32_t) _instances.techniques.size();
        _instances.techniques.push_back(technique);
        _instances.techniquesTable.insert({ material.name, techniqueIndex });

        // Ensure the number of techniques does not exceed the maximum allowed.
        assert(_instances.techniques.size() < MAX_TECHNIQUES);
    }
    return techniqueIndex;
}

// Loads and indexes a material. If the material is already loaded, returns its index.
gerium_uint32_t RenderService::getOrEmplaceMaterial(const MaterialData& material,
                                                    std::vector<MaterialNonCompressed>& materials) {
    // Compute a hash for the material data to enable efficient lookups.
    const auto hash = materialDataHash(material);

    gerium_uint32_t materialIndex = 0;

    // Check if the material is already in the cache.
    if (auto it = _instances.materialsTable.find(hash); it != _instances.materialsTable.end()) {
        materialIndex = it->second; // Use the cached index.
    } else {
        // Add the material to the cache and assign it a new index.
        materialIndex = (gerium_uint32_t) _instances.materialsTable.size();
        _instances.materialsTable.insert({ hash, materialIndex });

        // Helper lambda to load a texture and bind it if bindless textures are supported.
        auto loadTexture = [this](const entt::hashed_string& name) -> glm::uint {
            if (name.size()) {
                // Load the texture from the resource manager.
                Texture result = _resourceManager.loadTexture(name);
                _instances.textures.insert(result);
                gerium_texture_h texture = result;

                // If bindless textures are supported, bind the texture to the bindless array.
                if (_features.bindless) {
                    gerium_renderer_bind_texture(
                        _renderer, _bindlessTextures, BINDLESS_BINDING, texture.index, texture);
                }
                return texture.index; // Return the texture index.
            }

            // If the texture name is empty, use the empty texture as a fallback.
            gerium_texture_h emptyTexture = _emptyTexture;
            return emptyTexture.index;
        };

        materials.push_back({}); // Add a new material to the list.
        auto& mat = materials.back();

        // Fill the material data:
        mat.baseColorFactor[0]       = material.baseColorFactor.x; // Base color (R).
        mat.baseColorFactor[1]       = material.baseColorFactor.y; // Base color (G).
        mat.baseColorFactor[2]       = material.baseColorFactor.z; // Base color (B).
        mat.baseColorFactor[3]       = material.baseColorFactor.w; // Base color (A).
        mat.emissiveFactor[0]        = material.emissiveFactor.x;  // Emissive color (R).
        mat.emissiveFactor[1]        = material.emissiveFactor.y;  // Emissive color (G).
        mat.emissiveFactor[2]        = material.emissiveFactor.z;  // Emissive color (B).
        mat.metallicFactor           = material.metallicFactor;    // Metallic factor.
        mat.roughnessFactor          = material.roughnessFactor;   // Roughness factor.
        mat.occlusionStrength        = material.occlusionStrength; // Occlusion strength.
        mat.alphaCutoff              = material.alphaCutoff;       // Alpha cutoff.
        mat.baseColorTexture         = loadTexture(material.baseColorTexture);
        mat.metallicRoughnessTexture = loadTexture(material.metallicRoughnessTexture);
        mat.normalTexture            = loadTexture(material.normalTexture);
        mat.occlusionTexture         = loadTexture(material.occlusionTexture);
        mat.emissiveTexture          = loadTexture(material.emissiveTexture);
    }
    return materialIndex;
}

std::vector<gerium_uint8_t> RenderService::prepareMeshes(const Cluster& cluster) const {
    std::vector<gerium_uint8_t> buffer;
    // If 8-bit and 16-bit storage is supported, compress mesh data for efficiency.
    if (_features._8and16BitStorage) {
        size_t offset = 0;
        buffer.resize(cluster.meshes.size() * sizeof(Mesh));
        for (const auto& m : cluster.meshes) {
            Mesh result;
            result.center[0]    = meshopt_quantizeHalf(m.center[0]); // Quantize bounding sphere center.
            result.center[1]    = meshopt_quantizeHalf(m.center[1]);
            result.center[2]    = meshopt_quantizeHalf(m.center[2]);
            result.radius       = meshopt_quantizeHalf(m.radius);     // Quantize bounding sphere radius.
            result.bboxMin[0]   = meshopt_quantizeHalf(m.bboxMin[0]); // Quantize AABB min bounds.
            result.bboxMin[1]   = meshopt_quantizeHalf(m.bboxMin[1]);
            result.bboxMin[2]   = meshopt_quantizeHalf(m.bboxMin[2]);
            result.bboxMax[0]   = meshopt_quantizeHalf(m.bboxMax[0]); // Quantize AABB max bounds.
            result.bboxMax[1]   = meshopt_quantizeHalf(m.bboxMax[1]);
            result.bboxMax[2]   = meshopt_quantizeHalf(m.bboxMax[2]);
            result.vertexOffset = m.vertexOffset;      // Store vertex offset.
            result.vertexCount  = m.vertexCount;       // Store vertex count.
            result.lodCount     = uint8_t(m.lodCount); // Store LOD count.
            for (int l = 0; l < std::size(m.lods); ++l) {
                result.lods[l] = m.lods[l]; // Store LOD data.
            }
            memcpy(buffer.data() + offset, &result, sizeof(result));
            offset += sizeof(result);
        }
    } else {
        // Use uncompressed mesh data if compression is not supported.
        buffer.resize(cluster.meshes.size() * sizeof(cluster.meshes[0]));
        memcpy(buffer.data(), cluster.meshes.data(), buffer.size());
    }
    return buffer;
}

std::vector<gerium_uint8_t> RenderService::prepareVertices(const Cluster& cluster) const {
    std::vector<gerium_uint8_t> buffer;
    // If 8-bit and 16-bit storage is supported, compress vertex data for efficiency.
    if (_features._8and16BitStorage) {
        size_t offset = 0;
        buffer.resize(cluster.vertices.size() * sizeof(Vertex));
        for (const auto& v : cluster.vertices) {
            const auto n = glm::vec3(v.nx, v.ny, v.nz) * 127.0f + 127.5f; // Normalize and quantize normals.
            const auto t = glm::vec3(v.tx, v.ty, v.tz) * 127.0f + 127.5f; // Normalize and quantize tangents.
            const auto s = v.ts < 0.0f ? -1 : 1;                          // Determine tangent sign.

            Vertex result;
            result.px = meshopt_quantizeHalf(v.px); // Quantize vertex position.
            result.py = meshopt_quantizeHalf(v.py);
            result.pz = meshopt_quantizeHalf(v.pz);
            result.nx = uint8_t(n.x); // Store quantized normal.
            result.ny = uint8_t(n.y);
            result.nz = uint8_t(n.z);
            result.tx = uint8_t(t.x); // Store quantized tangent.
            result.ty = uint8_t(t.y);
            result.tz = uint8_t(t.z);
            result.ts = int8_t(s);                  // Store tangent sign.
            result.tu = meshopt_quantizeHalf(v.tu); // Quantize texture coordinates.
            result.tv = meshopt_quantizeHalf(v.tv);
            memcpy(buffer.data() + offset, &result, sizeof(result));
            offset += sizeof(result);
        }
    } else {
        // Use uncompressed vertices data if compression is not supported.
        buffer.resize(cluster.vertices.size() * sizeof(cluster.vertices[0]));
        memcpy(buffer.data(), cluster.vertices.data(), buffer.size());
    }
    return buffer;
}

std::span<const gerium_uint8_t> RenderService::prepareIndices(const Cluster& cluster) const {
    return std::span{ (const gerium_uint8_t*) cluster.indices.data(),
                      cluster.indices.size() * sizeof(cluster.indices[0]) };
}

std::span<const gerium_uint8_t> RenderService::prepareInstances(std::vector<MeshInstance>& instances,
                                                                gerium_uint32_t maxCount) {
    assert(maxCount < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);
    instances.resize(maxCount);
    return { (gerium_uint8_t*) instances.data(), instances.size() * sizeof(MeshInstance) };
}

std::span<const gerium_uint8_t> RenderService::prepareMaterials(std::vector<MaterialNonCompressed>& materials,
                                                                std::vector<Material>& tmp,
                                                                gerium_uint32_t count,
                                                                gerium_uint32_t maxCount) {
    assert(maxCount < MAX_TECHNIQUES * MAX_INSTANCES_PER_TECHNIQUE);
    if (_features._8and16BitStorage) {
        tmp.resize(maxCount);
        for (gerium_uint32_t i = 0; i < count; ++i) {
            const auto& material = materials[i];

            tmp[i].baseColorFactor[0]       = meshopt_quantizeHalf(material.baseColorFactor[0]); // Base color (R).
            tmp[i].baseColorFactor[1]       = meshopt_quantizeHalf(material.baseColorFactor[1]); // Base color (G).
            tmp[i].baseColorFactor[2]       = meshopt_quantizeHalf(material.baseColorFactor[2]); // Base color (B).
            tmp[i].baseColorFactor[3]       = meshopt_quantizeHalf(material.baseColorFactor[3]); // Base color (A).
            tmp[i].emissiveFactor[0]        = meshopt_quantizeHalf(material.emissiveFactor[0]);  // Emissive color (R).
            tmp[i].emissiveFactor[1]        = meshopt_quantizeHalf(material.emissiveFactor[1]);  // Emissive color (G).
            tmp[i].emissiveFactor[2]        = meshopt_quantizeHalf(material.emissiveFactor[2]);  // Emissive color (B).
            tmp[i].metallicFactor           = meshopt_quantizeHalf(material.metallicFactor);     // Metallic factor.
            tmp[i].roughnessFactor          = meshopt_quantizeHalf(material.roughnessFactor);    // Roughness factor.
            tmp[i].occlusionStrength        = meshopt_quantizeHalf(material.occlusionStrength);  // Occlusion strength.
            tmp[i].alphaCutoff              = meshopt_quantizeHalf(material.alphaCutoff);        // Alpha cutoff.
            tmp[i].baseColorTexture         = uint16_t(material.baseColorTexture);
            tmp[i].metallicRoughnessTexture = uint16_t(material.metallicRoughnessTexture);
            tmp[i].normalTexture            = uint16_t(material.normalTexture);
            tmp[i].occlusionTexture         = uint16_t(material.occlusionTexture);
            tmp[i].emissiveTexture          = uint16_t(material.emissiveTexture);
        }
        return { (gerium_uint8_t*) tmp.data(), tmp.size() * sizeof(Material) };
    } else {
        materials.resize(maxCount);
        return { (gerium_uint8_t*) materials.data(), materials.size() * sizeof(MaterialNonCompressed) };
    }
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
