#ifndef RENDER_SERVICE_HPP
#define RENDER_SERVICE_HPP

#include "../Model.hpp"
#include "../ResourceManager.hpp"
#include "../components/Renderable.hpp"
#include "../components/Transform.hpp"
#include "ServiceManager.hpp"

class RenderPass;

class RenderService final : public Service {
public:
    // Returns the resource manager for managing assets
    [[nodiscard]] ResourceManager& resourceManager() noexcept;

    // Returns the absolute frame number (total frames rendered since start) and the current frame index.
    [[nodiscard]] gerium_uint64_t frame() const noexcept;      // Absolute frame count
    [[nodiscard]] gerium_uint32_t frameIndex() const noexcept; // Current frame index [0, _maxFramesInFlight)

    // Returns descriptor sets for GPU resources:
    // - sceneData: Camera and scene-related data (e.g., view matrices).
    // - clusterData: Cluster data (e.g., vertices, indices).
    // - instancesData: Per-instance data (e.g., transforms, materials).
    // - textures: Bindless texture array.
    [[nodiscard]] gerium_descriptor_set_h sceneData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h clusterData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h instancesData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h textures() const noexcept;

    // Returns the total number of instances in the current scene.
    [[nodiscard]] gerium_uint32_t instancesCount() const noexcept;

    // Returns the list of rendering techniques used in the current scene.
    [[nodiscard]] const std::vector<Technique>& techniques() const noexcept;

    // Checks if the GPU supports specific features:
    // - bindlessSupported: Whether bindless textures are supported.
    // - drawIndirectSupported: Whether indirect draw calls are supported.
    // - drawIndirectCountSupported: Whether indirect draw calls with count are supported.
    // - is8and16BitStorageSupported: Whether 8-bit and 16-bit storage is supported.
    [[nodiscard]] bool bindlessSupported() const noexcept;
    [[nodiscard]] bool drawIndirectSupported() const noexcept;
    [[nodiscard]] bool drawIndirectCountSupported() const noexcept;
    [[nodiscard]] bool is8and16BitStorageSupported() const noexcept;

    // Fallback data for compatibility when modern GPU features are not supported:
    // - compatSceneData: Scene data (e.g., camera matrices) for CPU-side processing.
    // - compatMeshes: Mesh data for CPU-side processing.
    // - compatMaterials: Material data for CPU-side processing.
    // - compatInstances: Instance data for CPU-side processing.
    [[nodiscard]] const SceneData& compatSceneData() const noexcept;
    [[nodiscard]] const std::vector<MeshNonCompressed>& compatMeshes() const noexcept;
    [[nodiscard]] const std::vector<MaterialNonCompressed>& compatMaterials() const noexcept;
    [[nodiscard]] const std::vector<MeshInstance>& compatInstances() const noexcept;

    // Merges dynamic instances (e.g., moving objects) into the current instances buffer.
    // This is necessary for rendering objects efficiently.
    void mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer);

protected:
    // Constants for event handling in the ECS (Entity-Component-System) framework.
    static constexpr entt::hashed_string STORAGE_CONSTRUCT = { "create_renderable" };
    static constexpr entt::hashed_string STORAGE_DESTROY   = { "destroy_renderable" };
    static constexpr entt::hashed_string STORAGE_UPDATE    = { "update_renderable" };

    // Threshold for rebuilding the cluster after deletions
    static constexpr gerium_uint32_t deletionsToCheckUpdateCluster = 10;

    // Represents the geometry data for the current scene, including vertices, indices, and meshes.
    struct ClusterData {
        Buffer vertices;  // Linear vertex data for all geometry
        Buffer indices;   // Linear index data for all geometry
        Buffer meshes;    // Linear mesh and LOD data for all geometry
        DescriptorSet ds; // Descriptor set for accessing these buffers
    };

    // Represents the mapping between model nodes and their corresponding mesh indices.
    struct MeshIndices {
        struct Node {
            gerium_sint32_t nodeIndex; // Index of the node in the model
            gerium_uint32_t meshIndex; // Index of the mesh in the cluster
        };

        std::vector<Node> indices; // List of node-to-mesh mappings
    };

    // Adds a rendering pass to the frame graph.
    template <typename RP, typename... Args>
    void addPass(Args&&... args);

    // Lifecycle methods for starting, stopping, and updating the render service.
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    void reloadCluster();                       // Reload cluster
    void createCluster(const Cluster& cluster); // Creates a new cluster from the provided geometry
    void createStaticInstances();               // Creates static instances (e.g., non-moving objects)

    void updateCluster();          // Updates the cluster data (e.g., after deletions or changes)
    void updateActiveSceneData();  // Updates scene data (e.g., camera matrices)
    void updateDynamicInstances(); // Updates dynamic instances (e.g., moving objects)
    void updateInstancesData();    // Updates bindings of instances

    // Check for update cluster or static instances
    void checkNewComponents(bool& needReloadCluster, bool& needUpdateStaticInstances);
    void checkDestroyedComponents(bool& needReloadCluster, bool& needUpdateStaticInstances);
    void checkUpdatedComponents(bool& needReloadCluster, bool& needUpdateStaticInstances);

    // Helper methods for managing instances and materials:
    // - getInstances: Retrieves instances (static or dynamic).
    // - emplaceInstance Adds an instance to the list (instances).
    // - getOrEmplaceTechnique: Retrieves index or creates a technique for a material
    // - getOrEmplaceMaterial: Retrieves index or creates a material
    std::vector<MeshInstance> getInstances(bool isStatic);
    void emplaceInstance(const Renderable& renderable,
                         const Transform& transform,
                         std::vector<MeshInstance>& instances);
    gerium_uint32_t getOrEmplaceTechnique(const MaterialData& material);
    gerium_uint32_t getOrEmplaceMaterial(const MaterialData& material);

    // Computes a hash for material data to enable efficient lookups.
    static gerium_uint64_t materialDataHash(const MaterialData& material) noexcept;

    static gerium_uint32_t prepare(gerium_frame_graph_t frameGraph,
                                   gerium_renderer_t renderer,
                                   gerium_uint32_t maxWorkers,
                                   gerium_data_t data);
    static gerium_bool_t resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer, gerium_data_t data);
    static gerium_bool_t render(gerium_frame_graph_t frameGraph,
                                gerium_renderer_t renderer,
                                gerium_command_buffer_t commandBuffer,
                                gerium_uint32_t worker,
                                gerium_uint32_t totalWorkers,
                                gerium_data_t data);

    // Stores exceptions from C-callbacks for error handling.
    std::exception_ptr _error{};

    // Handles to the renderer and profiler for managing rendering and performance metrics.
    gerium_renderer_t _renderer{};
    gerium_profiler_t _profiler{};

    // Handle to the frame graph for managing rendering passes.
    gerium_frame_graph_t _frameGraph{};

    // Flags indicating supported GPU features:
    bool _bindlessSupported{};          // Whether bindless textures are supported
    bool _drawIndirectSupported{};      // Whether indirect draw calls are supported
    bool _drawIndirectCountSupported{}; // Whether indirect draw calls with count are supported
    bool _8and16BitStorageSupported{};  // Whether 8-bit and 16-bit storage is supported
    bool _isModernPipeline{}; // Whether all modern features are supported (_bindlessSupported && _drawIndirectSupported
                              // && _drawIndirectCountSupported)

    // Frame management:
    gerium_uint32_t _maxFramesInFlight{}; // Number of frames in flight (for multi-buffering)
    gerium_uint32_t _frameIndex{};        // Current frame index [0, _maxFramesInFlight)
    gerium_uint64_t _frame{};             // Absolute frame count
    gerium_uint16_t _width{};             // Current screen width
    gerium_uint16_t _height{};            // Current screen height

    // Resource management:
    ResourceManager _resourceManager{}; // Manages assets like textures, buffers, etc

    // Rendering passes:
    std::vector<std::unique_ptr<RenderPass>> _renderPasses{}; // List of rendering passes
    std::map<uint32_t, RenderPass*> _renderPassesCache{};     // Cache for quick access to rendering passes

    // Textures:
    Texture _emptyTexture{};           // Placeholder texture for fallback cases
    DescriptorSet _bindlessTextures{}; // Bindless texture array (if supported)

    // Geometry data:
    ClusterData _cluster{};                                        // Current geometry data (vertices, indices, meshes)
    std::map<entt::hashed_string, MeshIndices> _modelsInCluster{}; // Mapping of model names to mesh indices

    // Counters for tracking deletions:
    gerium_uint32_t _renderableDestroyed{}; // Number of renderable entities destroyed
    gerium_uint32_t _modelsDestroyed{};     // Number of models destroyed

    // Scene data:
    Buffer _activeScene{};            // GPU buffer for scene data (e.g., camera matrices)
    DescriptorSet _activeSceneData{}; // Descriptor set for scene data

    // Draw and instances data:
    Buffer _drawData{};               // GPU buffer for draw data (e.g., draw counts)
    Buffer _dynamicInstances{};       // GPU buffer for dynamic instances
    Buffer _dynamicMaterials{};       // GPU buffer for dynamic materials
    std::vector<Buffer> _instances{}; // GPU buffers for all instances (static and dynamic)
    std::vector<Buffer> _materials{}; // GPU buffers for all materials (static and dynamic)
    DescriptorSet _instancesData{};   // Descriptor set for draw data, instances, and materials

    // Counters for instances and materials:
    gerium_uint32_t _staticInstancesCount{};  // Number of static instances
    gerium_uint32_t _dynamicInstancesCount{}; // Number of dynamic instances
    gerium_uint32_t _staticMaterialsCount{};  // Number of static materials
    gerium_uint32_t _dynamicMaterialsCount{}; // Number of dynamic materials

    // Caches for techniques and materials:
    std::vector<Technique> _techniques{};                          // List of rendering techniques
    std::set<Texture> _textures{};                                 // Set of textures to prevent unloading
    std::map<gerium_uint32_t, gerium_uint32_t> _techniquesTable{}; // Mapping of technique IDs to indices
    std::map<gerium_uint64_t, gerium_uint32_t> _materialsTable{};  // Mapping of material hashes to indices
    std::vector<Material> _materialsCompressed{};                  // Compressed material data (if supported)
    std::vector<Material> _materialsNonCompressed{};               // Uncompressed material data (fallback)

    // Compatibility data for CPU-side processing (used when modern GPU features are not supported):
    SceneData _compatSceneData{};                          // Scene data for CPU-side processing
    std::vector<MeshNonCompressed> _compatMeshes{};        // Mesh data for CPU-side processing
    std::vector<MaterialNonCompressed> _compatMaterials{}; // Material data for CPU-side processing
    std::vector<MeshInstance> _compatInstances{};          // Instance data for CPU-side processing
};

inline ResourceManager& RenderService::resourceManager() noexcept {
    return _resourceManager;
}

inline gerium_uint64_t RenderService::frame() const noexcept {
    return _frame;
}

inline gerium_uint32_t RenderService::frameIndex() const noexcept {
    return _frameIndex;
}

inline gerium_descriptor_set_h RenderService::sceneData() const noexcept {
    return _activeSceneData;
}

inline gerium_descriptor_set_h RenderService::clusterData() const noexcept {
    return _cluster.ds;
}

inline gerium_descriptor_set_h RenderService::instancesData() const noexcept {
    return _instancesData;
}

inline gerium_descriptor_set_h RenderService::textures() const noexcept {
    assert(_bindlessSupported && "Only available if bindless is supported");
    return _bindlessTextures;
}

inline gerium_uint32_t RenderService::instancesCount() const noexcept {
    return _staticInstancesCount + _dynamicInstancesCount;
}

inline const std::vector<Technique>& RenderService::techniques() const noexcept {
    return _techniques;
}

inline bool RenderService::bindlessSupported() const noexcept {
    return _bindlessSupported;
}

inline bool RenderService::drawIndirectSupported() const noexcept {
    return _drawIndirectSupported;
}

inline bool RenderService::drawIndirectCountSupported() const noexcept {
    return _drawIndirectCountSupported;
}

inline bool RenderService::is8and16BitStorageSupported() const noexcept {
    return _8and16BitStorageSupported;
}

inline const SceneData& RenderService::compatSceneData() const noexcept {
    assert(_drawIndirectCountSupported == false && "Only available if indirect rendering is not supported.");
    return _compatSceneData;
}

inline const std::vector<MeshNonCompressed>& RenderService::compatMeshes() const noexcept {
    assert(_drawIndirectCountSupported == false && "Only available if indirect rendering is not supported.");
    return _compatMeshes;
}

inline const std::vector<MaterialNonCompressed>& RenderService::compatMaterials() const noexcept {
    assert(_bindlessSupported == false && "Only available if bindless is not supported");
    return _compatMaterials;
}

inline const std::vector<MeshInstance>& RenderService::compatInstances() const noexcept {
    assert(_drawIndirectCountSupported == false && "Only available if indirect rendering is not supported.");
    return _compatInstances;
}

template <typename RP, typename... Args>
inline void RenderService::addPass(Args&&... args) {
    static_assert(std::is_base_of_v<RenderPass, RP>);

    auto renderPass = std::make_unique<RP>(std::forward<Args>(args)...);
    renderPass->setRenderService(this);

    gerium_render_pass_t pass{ prepare, resize, render };
    gerium_frame_graph_add_pass(_frameGraph, renderPass->name().c_str(), &pass, renderPass.get());

    _renderPassesCache[entt::type_index<RP>::value()] = renderPass.get();
    _renderPasses.push_back(std::move(renderPass));
}

#endif
