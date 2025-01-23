#ifndef RENDER_PASS_HPP
#define RENDER_PASS_HPP

#include "../Common.hpp"
#include "../services/RenderService.hpp"

class RenderService;

class RenderPass : NonMovable {
public:
    explicit RenderPass(const std::string& name);

    virtual ~RenderPass() = default;

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

    void setRenderService(RenderService* service) noexcept;
    RenderService& renderService() noexcept;

    const std::string& name() const noexcept;

private:
    RenderService* _service{};
    const std::string _name;
};

#endif
