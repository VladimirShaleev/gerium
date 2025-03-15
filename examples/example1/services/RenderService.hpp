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
    // Returns an object for resource management
    [[nodiscard]] ResourceManager& resourceManager() noexcept;

    // Returns the frame number (absolute) and frame index
    [[nodiscard]] gerium_uint64_t frame() const noexcept;
    [[nodiscard]] gerium_uint32_t frameIndex() const noexcept;

    // Return descriptor sets of with binded resources for: camera, cluster
    // with all geometries, instances and materials, as well as bindless
    // textures (if supported).Also returns a list of techniques in the current scene.
    [[nodiscard]] gerium_descriptor_set_h sceneData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h clusterData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h instancesData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h textures() const noexcept;
    [[nodiscard]] gerium_uint32_t instancesCount() const noexcept;
    [[nodiscard]] const std::vector<Technique>& techniques() const noexcept;

    // Checking supported features
    [[nodiscard]] bool bindlessSupported() const noexcept;
    [[nodiscard]] bool drawIndirectSupported() const noexcept;
    [[nodiscard]] bool drawIndirectCountSupported() const noexcept;
    [[nodiscard]] bool is8and16BitStorageSupported() const noexcept;

    // If the device does not support modern rendering, we also save data on the CPU side
    // to support compatibility, which can be obtained through these methods. Then we
    // can use this data for legacy rendering with draw calls.
    [[nodiscard]] const SceneData& compatSceneData() const noexcept;
    [[nodiscard]] const std::vector<MeshNonCompressed>& compatMeshes() const noexcept;
    [[nodiscard]] const std::vector<MaterialNonCompressed>& compatMaterials() const noexcept;
    [[nodiscard]] const std::vector<MeshInstance>& compatInstances() const noexcept;

    // Merges dynamic instances buffer into the current instances buffer
    void mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer);

protected:
    static constexpr entt::hashed_string STORAGE_CONSTRUCT         = { "create_renderable" };
    static constexpr entt::hashed_string STORAGE_DESTROY           = { "destroy_renderable" };
    static constexpr entt::hashed_string STORAGE_UPDATE            = { "update_renderable" };
    static constexpr gerium_uint32_t deletionsToCheckUpdateCluster = 10;

    // Packed geometry of the current scene (unique models and LODs)
    struct ClusterData {
        Buffer vertices;  // linear vertex data for all geometry
        Buffer indices;   // linear index data for all geometry
        Buffer meshes;    // linear mesh and lods data for all geometry
        DescriptorSet ds; // a set of these buffers
    };

    // A structure for indexing GPU meshes on a CPU for model nodes
    struct MeshIndices {
        struct Node {
            gerium_sint32_t nodeIndex;
            gerium_uint32_t meshIndex;
        };

        std::vector<Node> indices;
    };

    // Adds a pass to the frame graph
    template <typename RP, typename... Args>
    void addPass(Args&&... args);

    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    void createCluster(const Cluster& cluster);
    void createStaticInstances();

    void updateCluster();
    void updateActiveSceneData();
    void updateDynamicInstances();
    void updateInstancesData();

    std::vector<MeshInstance> getInstances(bool isStatic);
    void emplaceInstance(const Renderable& renderable,
                         const Transform& transform,
                         std::vector<MeshInstance>& instances);
    gerium_uint32_t getOrEmplaceTechnique(const MaterialData& material);
    gerium_uint32_t getOrEmplaceMaterial(const MaterialData& material);

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

    // In case of an error from the C-callback, we will save the exception here
    std::exception_ptr _error{};

    // Objects for manipulating the render and getting data from the profiler
    gerium_renderer_t _renderer{};
    gerium_profiler_t _profiler{};

    // Object for working with frame graph
    gerium_frame_graph_t _frameGraph{};

    // Supported features of the selected GPU
    bool _bindlessSupported{};
    bool _drawIndirectSupported{};
    bool _drawIndirectCountSupported{};
    bool _8and16BitStorageSupported{};
    bool _isModernPipeline{}; // _bindlessSupported && _drawIndirectSupported && _drawIndirectCountSupported

    gerium_uint32_t _maxFramesInFlight{}; // number of frames in flight
    gerium_uint32_t _frameIndex{};        // current frame index [0.._maxFramesInFlight)
    gerium_uint64_t _frame{};             // absolute number of frames
    gerium_uint16_t _width{};             // current width
    gerium_uint16_t _height{};            // current height

    // Counting references to used resources
    ResourceManager _resourceManager{};

    // Rendering passes for frame graph
    std::vector<std::unique_ptr<RenderPass>> _renderPasses{};
    std::map<uint32_t, RenderPass*> _renderPassesCache{};

    Texture _emptyTexture{};           // placeholder in case texture is not set
    DescriptorSet _bindlessTextures{}; // set of textures if _bindlessSupported is true

    // Current geometry data on GPU
    ClusterData _cluster{};
    // Mesh indices in the current cluster
    std::map<entt::hashed_string, MeshIndices> _modelsInCluster{};

    // We count the removed components so that when enough meshes in the cluster
    // are no longer used we can rebuild the cluster without those meshes
    gerium_uint32_t _renderableDestroyed{};
    gerium_uint32_t _modelsDestroyed{};

    Buffer _activeScene{};            // GPU data of SceneData (camera)
    DescriptorSet _activeSceneData{}; // SceneData

    Buffer _drawData{};               // GPU data of the DrawData structure
    Buffer _dynamicInstances{};       // GPU data of the dynamic instances
    Buffer _dynamicMaterials{};       // GPU data of the materials for dynamic instances
    std::vector<Buffer> _instances{}; // GPU data of all instances (static and dynamic)
    std::vector<Buffer> _materials{}; // GPU data of all materials (static and dynamic)
    DescriptorSet _instancesData{};   // DrawData, commands buffer, instances and materials

    gerium_uint32_t _staticInstancesCount{};
    gerium_uint32_t _dynamicInstancesCount{};
    gerium_uint32_t _staticMaterialsCount{};
    gerium_uint32_t _dynamicMaterialsCount{};

    // Cache and indexing of techniques and materials
    std::vector<Technique> _techniques{};
    std::set<Texture> _textures{}; // keeping references to textures so they don't get unloaded from memory
    std::map<gerium_uint32_t, gerium_uint32_t> _techniquesTable{};
    std::map<gerium_uint64_t, gerium_uint32_t> _materialsTable{};
    std::vector<Material> _materialsCompressed{};
    std::vector<Material> _materialsNonCompressed{};

    // In modern rendering these vectors will be empty, they are
    // filled if a particular feature is not supported
    SceneData _compatSceneData{};
    std::vector<MeshNonCompressed> _compatMeshes{};
    std::vector<MaterialNonCompressed> _compatMaterials{};
    std::vector<MeshInstance> _compatInstances{};
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
