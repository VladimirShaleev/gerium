#ifndef RENDER_SERVICE_HPP
#define RENDER_SERVICE_HPP

#include "../Model.hpp"
#include "../ResourceManager.hpp"
#include "../components/Renderable.hpp"
#include "../components/WorldTransform.hpp"
#include "ServiceManager.hpp"

class RenderPass;

class RenderService : public Service {
public:
    [[nodiscard]] ResourceManager& resourceManager() noexcept;

    [[nodiscard]] gerium_uint64_t frame() const noexcept;
    [[nodiscard]] gerium_uint32_t frameIndex() const noexcept;

    [[nodiscard]] gerium_technique_h baseTechnique() const noexcept;

    [[nodiscard]] gerium_descriptor_set_h sceneData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h clusterData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h instancesData() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h textures() const noexcept;
    [[nodiscard]] gerium_uint32_t instancesCount() const noexcept;
    [[nodiscard]] const std::vector<Technique>& techniques() const noexcept;

    void createCluster(const Cluster& cluster);
    void createStaticInstances();
    void mergeStaticAndDynamicInstances(gerium_command_buffer_t commandBuffer);

protected:
    void start() override;
    void stop() override;
    void update(gerium_uint64_t elapsedMs, gerium_float64_t elapsed) override;

private:
    struct ClusterData {
        Buffer vertices;
        Buffer indices;
        Buffer meshes;
        DescriptorSet ds;
    };

    template <typename RP, typename... Args>
    void addPass(Args&&... args) {
        static_assert(std::is_base_of_v<RenderPass, RP>);

        auto renderPass = std::make_unique<RP>(std::forward<Args>(args)...);
        renderPass->setRenderService(this);

        gerium_render_pass_t pass{ prepare, resize, render };
        gerium_frame_graph_add_pass(_frameGraph, renderPass->name().c_str(), &pass, renderPass.get());

        _renderPassesCache[entt::type_index<RP>::value()] = renderPass.get();
        _renderPasses.push_back(std::move(renderPass));
    }

    void updateActiveSceneData();
    void updateDynamicInstances();
    void updateInstancesData();

    void getMaterialsAndInstances(const Renderable& renderable,
                                  const WorldTransform& worldTransform,
                                  std::vector<Material>& materials,
                                  std::vector<MeshInstance>& instances);

    std::pair<gerium_uint32_t, gerium_uint32_t> getMaterial(const MaterialData& material,
                                                            std::vector<Material>& result);

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
    std::vector<Buffer> _instances{};
    std::vector<Buffer> _materials{};
    gerium_uint32_t _staticInstancesCount{};
    gerium_uint32_t _dynamicInstancesCount{};
    gerium_uint32_t _staticMaterialsCount{};
    gerium_uint32_t _dynamicMaterialsCount{};
    std::map<gerium_uint32_t, gerium_uint32_t> _techniquesTable{};
    std::map<gerium_uint64_t, gerium_uint32_t> _materialsTable{};
    std::vector<Material> _dynamicMaterialsCache{};
    std::vector<Technique> _techniques{};
    std::set<Texture> _textures{};
    DescriptorSet _instancesData{};

    Buffer _activeScene{};
    DescriptorSet _activeSceneData{};

    std::vector<std::unique_ptr<RenderPass>> _renderPasses{};
    std::map<uint32_t, RenderPass*> _renderPassesCache{};
};

#endif
