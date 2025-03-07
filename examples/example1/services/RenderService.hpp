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
    [[nodiscard]] ResourceManager& resourceManager() noexcept;

    [[nodiscard]] gerium_uint64_t frame() const noexcept;
    [[nodiscard]] gerium_uint32_t frameIndex() const noexcept;

    [[nodiscard]] gerium_descriptor_set_h sceneData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h clusterData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h instancesData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h textures() const noexcept;
    [[nodiscard]] gerium_uint32_t instancesCount() const noexcept;
    [[nodiscard]] const std::vector<Technique>& techniques() const noexcept;

    [[nodiscard]] bool bindlessSupported() const noexcept;
    [[nodiscard]] bool drawIndirectSupported() const noexcept;
    [[nodiscard]] bool drawIndirectCountSupported() const noexcept;
    [[nodiscard]] bool is8and16BitStorageSupported() const noexcept;

    [[nodiscard]] const SceneData& compatSceneData() const noexcept;
    [[nodiscard]] const std::vector<MeshNonCompressed>& compatMeshes() const noexcept;
    [[nodiscard]] const std::vector<MeshInstance>& compatInstances() const noexcept;

    void mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer);

protected:
    static constexpr entt::hashed_string STORAGE_CONSTRUCT         = { "create_renderable" };
    static constexpr entt::hashed_string STORAGE_DESTROY           = { "destroy_renderable" };
    static constexpr gerium_uint32_t deletionsToCheckUpdateCluster = 1;

    struct ClusterData {
        Buffer vertices;
        Buffer indices;
        Buffer meshes;
        DescriptorSet ds;
    };

    struct MeshIndices {
        struct Node {
            gerium_sint32_t nodeIndex;
            gerium_uint32_t meshIndex;
        };

        std::vector<Node> indices;
    };

    template <typename RP, typename... Args>
    void addPass(Args&&... args);

    template <typename M, typename Pred>
    std::pair<gerium_uint32_t, gerium_uint32_t> getMaterial(const MaterialData& material,
                                                            std::vector<M>& result,
                                                            Pred&& pred);

    template <typename Pred, typename M>
    void getMaterialsAndInstances(const Renderable& renderable,
                                  const Transform& transform,
                                  std::vector<M>& materials,
                                  std::vector<MeshInstance>& instances,
                                  Pred&& pred);
    void getMaterialsAndInstances(const Renderable& renderable,
                                  const Transform& transform,
                                  std::vector<Material>& materials,
                                  std::vector<MeshInstance>& instances);
    void getMaterialsAndInstances(const Renderable& renderable,
                                  const Transform& transform,
                                  std::vector<MaterialNonCompressed>& materials,
                                  std::vector<MeshInstance>& instances);

    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

    void createCluster(const Cluster& cluster);
    void createStaticInstances();

    void updateCluster();
    void updateActiveSceneData();
    void updateDynamicInstances();
    void updateInstancesData();

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

    std::exception_ptr _error{};
    gerium_renderer_t _renderer{};
    gerium_profiler_t _profiler{};
    gerium_frame_graph_t _frameGraph{};
    bool _bindlessSupported{};
    bool _drawIndirectSupported{};
    bool _drawIndirectCountSupported{};
    bool _8and16BitStorageSupported{};
    bool _isModernPipeline{};
    gerium_uint32_t _maxFramesInFlight{};
    gerium_uint32_t _frameIndex{};
    gerium_uint64_t _frame{};
    gerium_uint16_t _width{};
    gerium_uint16_t _height{};
    ResourceManager _resourceManager{};
    Texture _emptyTexture{};
    DescriptorSet _bindlessTextures{};
    Technique _baseTechnique{};
    ClusterData _cluster{};

    Buffer _drawData{};
    Buffer _dynamicInstances{};
    Buffer _dynamicMaterials{};
    SceneData _compatSceneData{};
    std::vector<MeshNonCompressed> _compatMeshes{};
    std::vector<MeshInstance> _compatInstances{};
    std::vector<Buffer> _instances{};
    std::vector<Buffer> _materials{};
    gerium_uint32_t _staticInstancesCount{};
    gerium_uint32_t _dynamicInstancesCount{};
    gerium_uint32_t _staticMaterialsCount{};
    gerium_uint32_t _dynamicMaterialsCount{};
    std::map<gerium_uint32_t, gerium_uint32_t> _techniquesTable{};
    std::map<gerium_uint64_t, gerium_uint32_t> _materialsTable{};
    std::vector<MaterialNonCompressed> _compatDynamicMaterialsCache{};
    std::vector<Material> _dynamicMaterialsCache{};
    std::vector<Technique> _techniques{};
    std::set<Texture> _textures{};
    DescriptorSet _instancesData{};

    Buffer _activeScene{};
    DescriptorSet _activeSceneData{};

    std::vector<std::unique_ptr<RenderPass>> _renderPasses{};
    std::map<uint32_t, RenderPass*> _renderPassesCache{};

    std::map<entt::hashed_string, MeshIndices> _modelsInCluster{};
    gerium_uint32_t _renderableDestroyed{};
    gerium_uint32_t _modelsDestroyed{};
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
    return _compatSceneData;
}

inline const std::vector<MeshNonCompressed>& RenderService::compatMeshes() const noexcept {
    return _compatMeshes;
}

inline const std::vector<MeshInstance>& RenderService::compatInstances() const noexcept {
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

template <typename M, typename Pred>
inline std::pair<gerium_uint32_t, gerium_uint32_t> RenderService::getMaterial(const MaterialData& material,
                                                                              std::vector<M>& result,
                                                                              Pred&& pred) {
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
    }

    return { techniqueIndex, materialIndex };
}

template <typename Pred, typename M>
inline void RenderService::getMaterialsAndInstances(const Renderable& renderable,
                                                    const Transform& transform,
                                                    std::vector<M>& materials,
                                                    std::vector<MeshInstance>& instances,
                                                    Pred&& pred) {
    for (const auto& meshData : renderable.meshes) {
        const auto [techniqueIndex, materialIndex] =
            getMaterial<M, Pred>(meshData.material, materials, std::move(pred));

        instances.push_back({});
        auto& instance = instances.back();

        const auto scale = glm::max(glm::max(transform.scale.x, transform.scale.y), transform.scale.z);

        instance.world        = transform.matrix;
        instance.prevWorld    = transform.prevMatrix;
        instance.normalMatrix = glm::transpose(glm::inverse(transform.matrix));
        instance.scale        = scale;
        instance.mesh         = meshData.mesh;
        instance.technique    = techniqueIndex;
        instance.material     = materialIndex;
    }
}

inline void RenderService::getMaterialsAndInstances(const Renderable& renderable,
                                                    const Transform& transform,
                                                    std::vector<Material>& materials,
                                                    std::vector<MeshInstance>& instances) {
    getMaterialsAndInstances(renderable, transform, materials, instances, meshopt_quantizeHalf);
}

inline void RenderService::getMaterialsAndInstances(const Renderable& renderable,
                                                    const Transform& transform,
                                                    std::vector<MaterialNonCompressed>& materials,
                                                    std::vector<MeshInstance>& instances) {
    getMaterialsAndInstances(renderable, transform, materials, instances, [](const auto value) {
        return value;
    });
}

#endif
