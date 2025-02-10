#ifndef RENDER_SERVICE_HPP
#define RENDER_SERVICE_HPP

#include "../Model.hpp"
#include "../ResourceManager.hpp"
#include "ServiceManager.hpp"

class RenderPass;

class RenderService : public Service {
public:
    [[nodiscard]] gerium_technique_h baseTechnique() const noexcept;

    [[nodiscard]] ResourceManager& resourceManager() noexcept;

    // [[nodiscard]] gerium_descriptor_set_h getSceneDescriptorSet(Entity entity);
    // [[nodiscard]] gerium_descriptor_set_h getActiveSceneDescriptorSet();

    [[nodiscard]] gerium_descriptor_set_h clusterDescriptorSet() const noexcept;
    [[nodiscard]] gerium_descriptor_set_h sceneDataDescriptorSet() const noexcept;
    [[nodiscard]] gerium_buffer_h instancesBuffer() const noexcept;
    [[nodiscard]] gerium_buffer_h instanceCountBuffer() const noexcept;
    [[nodiscard]] gerium_uint32_t instanceCount() const noexcept;

    void loadModel(const std::string& filename);

    void createCluster(const Cluster& cluster);
    void createStaticInstances();

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
    ResourceManager _resourceManager{};
    Technique _baseTechnique{};
    ClusterData _cluster{};
    Buffer _staticInstances{};
    Buffer _staticInstanceCountBuff{};
    gerium_uint32_t _staticInstanceCount{};
    Buffer _activeCamera{};
    DescriptorSet _activeCameraDs{};

    std::vector<std::unique_ptr<RenderPass>> _renderPasses{};
    std::map<uint32_t, RenderPass*> _renderPassesCache{};
};

#endif
