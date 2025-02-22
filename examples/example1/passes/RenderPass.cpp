#include "RenderPass.hpp"
#include "../services/RenderService.hpp"

RenderPass::RenderPass(const std::string& name) : _name(name) {
}

void RenderPass::initialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
}

void RenderPass::uninitialize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
}

gerium_uint32_t RenderPass::prepare(gerium_frame_graph_t frameGraph,
                                    gerium_renderer_t renderer,
                                    gerium_uint32_t maxWorkers) {
    return 1;
}

void RenderPass::resize(gerium_frame_graph_t frameGraph, gerium_renderer_t renderer) {
}

void RenderPass::setRenderService(RenderService* service) noexcept {
    _service = service;
}

RenderService& RenderPass::renderService() noexcept {
    return *_service;
}

ResourceManager& RenderPass::resourceManager() noexcept {
    return renderService().resourceManager();
}

entt::registry& RenderPass::entityRegistry() noexcept {
    return _service->entityRegistry();
}

const entt::registry& RenderPass::entityRegistry() const noexcept {
    return _service->entityRegistry();
}

const std::string& RenderPass::name() const noexcept {
    return _name;
}
