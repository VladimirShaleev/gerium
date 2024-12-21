#ifndef RENDER_PASS_HPP
#define RENDER_PASS_HPP

#include "Common.hpp"

class Application;
class Settings;

class RenderPass {
public:
    explicit RenderPass(const std::string& name);

    virtual ~RenderPass() = default;

    virtual void registerResources(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer);
    virtual void initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer);
    virtual void uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer);

    virtual gerium_uint32_t prepare(gerium_frame_graph_t frameGraph,
                                    gerium_renderer_t renderer,
                                    gerium_uint32_t maxWorkers);
    virtual void resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer);
    virtual void render(gerium_frame_graph_t frameGraph,
                        gerium_renderer_t renderer,
                        gerium_command_buffer_t commandBuffer,
                        gerium_uint32_t worker,
                        gerium_uint32_t totalWorkers) = 0;

    void setApplication(Application* application) noexcept;
    Application* application() const noexcept;
    Settings& settings() noexcept;

    const std::string& name() const noexcept;

private:
    Application* _application{};
    const std::string _name;
};

#endif
