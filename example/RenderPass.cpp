#include "RenderPass.hpp"

RenderPass::RenderPass(const std::string& name) : _name(name) {
}

void RenderPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
}

void RenderPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
}

gerium_uint32_t RenderPass::prepare(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
    return 1;
}

void RenderPass::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
}

void RenderPass::setApplication(Application* application) noexcept {
    _application = application;
}

Application* RenderPass::getApplication() const noexcept {
    return _application;
}

const std::string& RenderPass::name() const noexcept {
    return _name;
}
