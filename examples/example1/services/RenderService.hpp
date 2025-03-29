#ifndef RENDER_SERVICE_HPP
#define RENDER_SERVICE_HPP

#include "../Model.hpp"
#include "../ResourceManager.hpp"
#include "../components/Renderable.hpp"
#include "../components/Transform.hpp"
#include "../events/DirtySceneEvent.hpp"
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
    // - scene: Camera and scene-related data (e.g., view matrices).
    // - cluster: Cluster data (e.g., vertices, indices).
    // - instances: Per-instance data (e.g., transforms, materials).
    // - textures: Bindless texture array.
    [[nodiscard]] gerium_descriptor_set_h scene() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h cluster() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h instances() const noexcept;
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
    // Specify supported GPU features
    struct Features {
        bool bindless;          // Whether bindless textures are supported
        bool drawIndirect;      // Whether indirect draw calls are supported
        bool drawIndirectCount; // Whether indirect draw calls with count are supported
        bool _8and16BitStorage; // Whether 8-bit and 16-bit storage is supported
        bool isModernPipeline;  // Whether all modern features are supported (bindless && drawIndirect &&
                                // drawIndirectCount)
    };

    // Frame data
    struct FrameData {
        gerium_uint32_t maxFramesInFlight; // Number of frames in flight (for multi-buffering)
        gerium_uint32_t frameIndex;        // Current frame index [0, _maxFramesInFlight)
        gerium_uint64_t frame;             // Absolute frame count
        gerium_uint16_t width;             // Current screen width
        gerium_uint16_t height;            // Current screen height
    };

    // Represents the mapping between model nodes and their corresponding mesh indices.
    struct ModelIndices {
        struct Node {
            gerium_sint32_t nodeIndex; // Index of the node in the model
            gerium_uint32_t meshIndex; // Index of the mesh in the cluster
        };

        std::vector<Node> nodes; // List of node-to-mesh mappings
    };

    // Represents the geometry data for the current scene, including vertices, indices, and meshes.
    struct ClusterData {
        typedef std::map<entt::hashed_string, ModelIndices> MapModelIndices;
        MapModelIndices models; // Mapping of model names to mesh indices
        Buffer vertices;        // Linear vertex data for all geometry
        Buffer indices;         // Linear index data for all geometry
        Buffer meshes;          // Linear mesh and LOD data for all geometry
        DescriptorSet ds;       // Descriptor set for accessing these buffers
    };

    // Scene data
    struct ActiveSceneData {
        Buffer buffer;    // GPU buffer for scene data (e.g., camera matrices)
        DescriptorSet ds; // Descriptor set for scene data
    };

    // Draw and instances data
    struct InstancesData {
        Buffer drawData;               // GPU buffer for draw data (e.g., draw counts)
        Buffer dynamicInstances;       // GPU buffer for dynamic instances
        Buffer dynamicMaterials;       // GPU buffer for dynamic materials
        std::vector<Buffer> instances; // GPU buffers for all instances (static and dynamic)
        std::vector<Buffer> materials; // GPU buffers for all materials (static and dynamic)
        DescriptorSet ds;              // Descriptor set for draw data, instances, and materials

        // Counters for instances and materials:
        gerium_uint32_t staticInstancesCount;  // Number of static instances
        gerium_uint32_t dynamicInstancesCount; // Number of dynamic instances
        gerium_uint32_t staticMaterialsCount;  // Number of static materials
        gerium_uint32_t dynamicMaterialsCount; // Number of dynamic materials

        // Caches for techniques, materials and textures:
        std::vector<Technique> techniques;                          // List of rendering techniques
        std::set<Texture> textures;                                 // Set of textures to prevent unloading
        std::map<gerium_uint32_t, gerium_uint32_t> techniquesTable; // Mapping of technique IDs to indices
        std::map<gerium_uint64_t, gerium_uint32_t> materialsTable;  // Mapping of material hashes to indices
        std::vector<MaterialNonCompressed> dynamicMaterialData;     // Materials for dynamic instances only

        bool isDirtyStatics; // Statics changed
    };

    // Compatibility data for CPU-side processing (used when modern GPU features are not supported)
    struct CompatData {
        SceneData sceneData{};                          // Scene data for CPU-side processing
        std::vector<MeshNonCompressed> meshes{};        // Mesh data for CPU-side processing
        std::vector<MaterialNonCompressed> materials{}; // Material data for CPU-side processing
        std::vector<MeshInstance> instances{};          // Instance data for CPU-side processing
    };

    // Adds a rendering pass to the frame graph.
    template <typename RP, typename... Args>
    void addPass(Args&&... args);

    // Marks statics as changed
    void onDirtyScene(const DirtySceneEvent& event);

    // Lifecycle methods for starting, stopping, and updating the render service.
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    void initRenderer(const gerium_renderer_options_t& options); // Create a renderer handle
    void initFrameGraph();                                       // Add render passes, load and compile Frame Graph
    void initTextures(gerium_uint32_t elementCount);             // Initialize empty texture and bindless textures
    void initData();                                             // Create GPU buffers and Descriptor Sets
    void initCluster();                                          // Load all geometry
    void updateActiveSceneData();                                // Updates scene data (e.g., camera matrices)
    void updateStaticInstances();                                // Update static instances (e.g., non-moving objects)
    void updateDynamicInstances();                               // Updates dynamic instances (e.g., moving objects)
    void updateInstancesData();                                  // Updates bindings of instances

    // Helper methods for managing instances and materials:
    // - getInstances: Retrieves instances (static or dynamic).
    // - emplaceInstance Adds an instance to the list (instances).
    // - getOrEmplaceTechnique: Retrieves index or creates a technique for a material
    // - getOrEmplaceMaterial: Retrieves index or creates a material
    void getInstances(bool isStatic,
                      std::vector<MeshInstance>& instances,
                      std::vector<MaterialNonCompressed>& materials);
    void emplaceInstance(const Renderable& renderable,
                         const Transform& transform,
                         std::vector<MeshInstance>& instances,
                         std::vector<MaterialNonCompressed>& materials);
    gerium_uint32_t getOrEmplaceTechnique(const MaterialData& material);
    gerium_uint32_t getOrEmplaceMaterial(const MaterialData& material, std::vector<MaterialNonCompressed>& materials);

    // Prepare data for GPU
    std::vector<gerium_uint8_t> prepareMeshes(const Cluster& cluster) const;
    std::vector<gerium_uint8_t> prepareVertices(const Cluster& cluster) const;
    std::span<const gerium_uint8_t> prepareIndices(const Cluster& cluster) const;
    std::span<const gerium_uint8_t> prepareInstances(std::vector<MeshInstance>& instances, gerium_uint32_t maxCount);
    std::span<const gerium_uint8_t> prepareMaterials(std::vector<MaterialNonCompressed>& materials,
                                                     std::vector<Material>& tmp,
                                                     gerium_uint32_t count,
                                                     gerium_uint32_t maxCount);

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

    std::exception_ptr _error{};        // Stores exceptions from C-callbacks for error handling.
    gerium_renderer_t _renderer{};      // Handles to the renderer for managing rendering.
    gerium_frame_graph_t _frameGraph{}; // Handle to the frame graph for managing rendering passes.
    ResourceManager _resourceManager{}; // Manages assets like textures, buffers, etc

    std::vector<std::unique_ptr<RenderPass>> _renderPasses{}; // List of rendering passes
    std::map<uint32_t, RenderPass*> _renderPassesCache{};     // Cache for quick access to rendering passes

    Features _features{};              // Specify supported GPU features
    Texture _emptyTexture{};           // Placeholder texture for fallback cases
    DescriptorSet _bindlessTextures{}; // Bindless texture array (if supported)
    FrameData _frame{};                // Frame data
    ClusterData _cluster{};            // Current geometry data (vertices, indices, meshes)
    ActiveSceneData _scene{};          // Active scene data (matrices, etc)
    InstancesData _instances{};        // Draw and instances data
    CompatData _compat{};              // Compatibility data for CPU-side processing
};

inline ResourceManager& RenderService::resourceManager() noexcept {
    return _resourceManager;
}

inline gerium_uint64_t RenderService::frame() const noexcept {
    return _frame.frame;
}

inline gerium_uint32_t RenderService::frameIndex() const noexcept {
    return _frame.frameIndex;
}

inline gerium_descriptor_set_h RenderService::scene() const noexcept {
    return _scene.ds;
}

inline gerium_descriptor_set_h RenderService::cluster() const noexcept {
    return _cluster.ds;
}

inline gerium_descriptor_set_h RenderService::instances() const noexcept {
    return _instances.ds;
}

inline gerium_descriptor_set_h RenderService::textures() const noexcept {
    assert(_features.bindless && "Only available if bindless is supported");
    return _bindlessTextures;
}

inline gerium_uint32_t RenderService::instancesCount() const noexcept {
    return _instances.staticInstancesCount + _instances.dynamicInstancesCount;
}

inline const std::vector<Technique>& RenderService::techniques() const noexcept {
    return _instances.techniques;
}

inline bool RenderService::bindlessSupported() const noexcept {
    return _features.bindless;
}

inline bool RenderService::drawIndirectSupported() const noexcept {
    return _features.drawIndirect;
}

inline bool RenderService::drawIndirectCountSupported() const noexcept {
    return _features.drawIndirectCount;
}

inline bool RenderService::is8and16BitStorageSupported() const noexcept {
    return _features._8and16BitStorage;
}

inline const SceneData& RenderService::compatSceneData() const noexcept {
    assert(_features.isModernPipeline == false &&
           "Only available if indirect rendering or bindless textures is not supported.");
    return _compat.sceneData;
}

inline const std::vector<MeshNonCompressed>& RenderService::compatMeshes() const noexcept {
    assert(_features.isModernPipeline == false &&
           "Only available if indirect rendering or bindless textures is not supported.");
    return _compat.meshes;
}

inline const std::vector<MaterialNonCompressed>& RenderService::compatMaterials() const noexcept {
    assert(_features.bindless == false && "Only available if bindless is not supported");
    return _compat.materials;
}

inline const std::vector<MeshInstance>& RenderService::compatInstances() const noexcept {
    assert(_features.isModernPipeline == false &&
           "Only available if indirect rendering or bindless textures is not supported.");
    return _compat.instances;
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
