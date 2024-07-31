#include "VkRenderer.hpp"
#include "../FrameGraph.hpp"

namespace gerium::vulkan {

VkRenderer::VkRenderer(Application* application, ObjectPtr<Device>&& device) noexcept :
    _application(application),
    _device(std::move(device)),
    _width(0),
    _height(0),
    _currentRenderPass(nullptr) {
    if (!application->isRunning()) {
        error(GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING);
    }
}

PipelineHandle VkRenderer::getPipeline(TechniqueHandle handle) const noexcept {
    auto technique = _techniques.access(handle);

    auto it = std::lower_bound(technique->passes,
                               technique->passes + technique->passCount,
                               _currentRenderPass,
                               [](const auto& p1, const auto& pass) {
        return p1.render_pass < pass;
    });

    return it != technique->passes + technique->passCount ? it->pipeline : Undefined;
}

void VkRenderer::onInitialize(gerium_uint32_t version, bool debug) {
    _device->create(application(), version, debug);
}

Application* VkRenderer::application() noexcept {
    return _application.get();
}

bool VkRenderer::onGetProfilerEnable() const noexcept {
    return _device->isProfilerEnable();
}

void VkRenderer::onSetProfilerEnable(bool enable) noexcept {
    _device->setProfilerEnable(enable);
}

BufferHandle VkRenderer::onCreateBuffer(const BufferCreation& creation) {
    return _device->createBuffer(creation);
}

TextureHandle VkRenderer::onCreateTexture(const TextureCreation& creation) {
    return _device->createTexture(creation);
}

TechniqueHandle VkRenderer::onCreateTechnique(const FrameGraph& frameGraph,
                                              gerium_utf8_t name,
                                              gerium_uint32_t pipelineCount,
                                              const gerium_pipeline_t* pipelines) {
    auto [handle, technique] = _techniques.obtain_and_access();
    technique->name          = intern(name);

    for (gerium_uint32_t i = 0; i < pipelineCount; ++i) {
        PipelineCreation pc{};
        pc.rasterization = pipelines[i].rasterization;
        pc.depthStencil  = pipelines[i].depth_stencil;
        pc.colorBlend    = pipelines[i].color_blend;
        pc.name          = pipelines[i].render_pass;

        if (auto node = frameGraph.getNode(pipelines[i].render_pass); node) {
            for (auto j = 0; j < node->outputCount; ++j) {
                auto& info = frameGraph.getResource(node->outputs[j])->info;
                if (info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT &&
                    !hasDepthOrStencil(toVkFormat(info.texture.format))) {
                    pc.blendState.addBlendState(info.texture.colorWriteMask, info.texture.colorBlend);
                }
            }
        }

        for (gerium_uint32_t j = 0; j < pipelines[i].shader_count; ++j) {
            pc.program.addStage(pipelines[i].shaders[j]);
        }
        pc.program.setName(pipelines[i].render_pass);

        for (gerium_uint32_t j = 0; j < pipelines[i].vertex_attribute_count; ++j) {
            const auto& attribute = pipelines[i].vertex_attributes[j];
            pc.vertexInput.addVertexAttribute(attribute);
        }

        for (gerium_uint32_t j = 0; j < pipelines[i].vertex_binding_count; ++j) {
            const auto& binding = pipelines[i].vertex_bindings[j];
            pc.vertexInput.addVertexBinding(binding);
        }

        if (auto node = frameGraph.getNode(pipelines[i].render_pass); node && node->renderPass != Undefined) {
            pc.renderPass = _device->getRenderPassOutput(node->renderPass);
        } else {
            pc.renderPass = _device->getRenderPassOutput(_device->getSwapchainPass());
        }

        technique->passes[i].render_pass = intern(pipelines[i].render_pass);
        technique->passes[i].pipeline    = _device->createPipeline(pc);
        ++technique->passCount;
    }

    std::sort(technique->passes, technique->passes + technique->passCount, [](const auto& mat1, const auto& mat2) {
        return mat1.render_pass < mat2.render_pass;
    });

    return handle;
}

DescriptorSetHandle VkRenderer::onCreateDescriptorSet() {
    DescriptorSetCreation creation{};
    return _device->createDescriptorSet(creation);
}

RenderPassHandle VkRenderer::onCreateRenderPass(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    RenderPassCreation creation{};
    creation.setName(node->name);

    for (gerium_uint32_t i = 0; i < node->outputCount; ++i) {
        auto& info = frameGraph.getResource(node->outputs[i])->info;

        if (info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
            const auto format = toVkFormat(info.texture.format);

            if (hasDepthOrStencil(format)) {
                creation.output.depth(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                creation.output.setDepthStencilOperations(info.texture.operation, info.texture.operation);
            } else {
                creation.output.color(format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, info.texture.operation);
            }
        }
    }

    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto& info = frameGraph.getResource(node->inputs[i])->info;

        if (info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
            const auto format = toVkFormat(info.texture.format);

            if (hasDepthOrStencil(format)) {
                creation.output.depth(format, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
                creation.output.setDepthStencilOperations(info.texture.operation, GERIUM_RENDER_PASS_OP_LOAD);
            } else {
                creation.output.color(format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, GERIUM_RENDER_PASS_OP_LOAD);
            }
        }
    }

    return _device->createRenderPass(creation);
}

FramebufferHandle VkRenderer::onCreateFramebuffer(const FrameGraph& frameGraph, const FrameGraphNode* node) {
    FramebufferCreation creation{};
    creation.renderPass = node->renderPass;
    creation.setName(node->name);

    gerium_uint16_t width  = 0;
    gerium_uint16_t height = 0;

    for (gerium_uint32_t i = 0; i < node->outputCount; ++i) {
        auto& info = frameGraph.getResource(node->outputs[i])->info;

        if (info.type == GERIUM_RESOURCE_TYPE_BUFFER || info.type == GERIUM_RESOURCE_TYPE_REFERENCE) {
            continue;
        }

        if (width == 0) {
            width = info.texture.width;
        } else {
            assert(width == info.texture.width);
        }

        if (height == 0) {
            height = info.texture.height;
        } else {
            assert(height == info.texture.height);
        }

        if (hasDepthOrStencil(toVkFormat(info.texture.format))) {
            // TODO: check 1 depth/stencil target
            creation.setDepthStencilTexture(info.texture.handle);
        } else {
            creation.addRenderTexture(info.texture.handle);
        }
    }

    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto inputResource = frameGraph.getResource(node->inputs[i]);

        if (inputResource->info.type == GERIUM_RESOURCE_TYPE_BUFFER ||
            inputResource->info.type == GERIUM_RESOURCE_TYPE_REFERENCE) {
            continue;
        }

        auto& info = frameGraph.getResource(inputResource->name)->info;

        if (width == 0) {
            width = info.texture.width;
        } else {
            assert(width == info.texture.width);
        }

        if (height == 0) {
            height = info.texture.height;
        } else {
            assert(height == info.texture.height);
        }

        if (inputResource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE) {
            continue;
        }

        if (hasDepthOrStencil(toVkFormat(info.texture.format))) {
            // TODO: check 1 depth/stencil target
            creation.setDepthStencilTexture(info.texture.handle);
        } else {
            creation.addRenderTexture(info.texture.handle);
        }
    }

    creation.width  = width;
    creation.height = height;

    return _device->createFramebuffer(creation);
}

void VkRenderer::onDestroyBuffer(BufferHandle handle) noexcept {
    _device->destroyBuffer(handle);
}

void VkRenderer::onDestroyTexture(TextureHandle handle) noexcept {
    _device->destroyTexture(handle);
}

void VkRenderer::onDestroyTechnique(TechniqueHandle handle) noexcept {
    auto meterial = _techniques.access(handle);

    for (gerium_uint32_t i = 0; i < meterial->passCount; ++i) {
        _device->destroyPipeline(meterial->passes[i].pipeline);
    }

    _techniques.release(handle);
}

void VkRenderer::onDestroyDescriptorSet(DescriptorSetHandle handle) noexcept {
    _device->destroyDescriptorSet(handle);
}

void VkRenderer::onDestroyRenderPass(RenderPassHandle handle) noexcept {
    _device->destroyRenderPass(handle);
}

void VkRenderer::onBind(DescriptorSetHandle handle, gerium_uint16_t binding, BufferHandle buffer) noexcept {
    _device->bind(handle, binding, buffer);
}

void VkRenderer::onBind(DescriptorSetHandle handle, gerium_uint16_t binding, gerium_utf8_t resourceInput) noexcept {
    _device->bind(handle, binding, Undefined, resourceInput);
}

gerium_data_t VkRenderer::onMapBuffer(BufferHandle handle, gerium_uint32_t offset, gerium_uint32_t size) noexcept {
    return _device->mapBuffer(handle, offset, size);
}

void VkRenderer::onUnmapBuffer(BufferHandle handle) noexcept {
    _device->unmapBuffer(handle);
}

void VkRenderer::onDestroyFramebuffer(FramebufferHandle handle) noexcept {
    _device->destroyFramebuffer(handle);
}

bool VkRenderer::onNewFrame() {
    return _device->newFrame();
}

void VkRenderer::onRender(FrameGraph& frameGraph) {
    gerium_uint16_t width, height;
    getSwapchainSize(width, height);

    if (_width != width || _height != height) {
        if (_width != 0 && _height != 0) {
            frameGraph.resize(_width, width, _height, height);
        }
        _width  = width;
        _height = height;
    }

    for (gerium_uint32_t i = 0; i < frameGraph.nodeCount(); ++i) {
        auto node = frameGraph.getNode(i);

        if (!node->enabled) {
            continue;
        }

        if (auto pass = frameGraph.getPass(node->pass); pass->pass.prepare) {
            pass->pass.prepare(alias_cast<gerium_frame_graph_t>(&frameGraph), this, pass->data);
        }
    }

    auto cb = _device->getCommandBuffer(0);
    cb->bindRenderer(this);
    cb->pushMarker("total");

    cb->pushMarker("frame_graph");
    for (gerium_uint32_t i = 0; i < frameGraph.nodeCount(); ++i) {
        auto node = frameGraph.getNode(i);

        if (!node->enabled) {
            continue;
        }

        auto pass = frameGraph.getPass(node->pass);

        _currentRenderPass = node->name;

        cb->pushMarker(node->name);

        gerium_uint16_t width;
        gerium_uint16_t height;

        for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
            auto resource = frameGraph.getResource(node->inputs[i]);

            if (resource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE) {
                cb->addImageBarrier(
                    resource->info.texture.handle, ResourceState::RenderTarget, ResourceState::ShaderResource, 0, 1);
            } else if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
                width  = resource->info.texture.width;
                height = resource->info.texture.height;
            }
        }

        for (gerium_uint32_t i = 0; i < node->outputCount; ++i) {
            auto resource = frameGraph.getResource(node->outputs[i]);

            if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
                width  = resource->info.texture.width;
                height = resource->info.texture.height;

                const auto format = toVkFormat(resource->info.texture.format);

                if (hasDepthOrStencil(format)) {
                    cb->addImageBarrier(
                        resource->info.texture.handle, ResourceState::Undefined, ResourceState::DepthWrite, 0, 1);
                    cb->clearDepthStencil(resource->info.texture.clearDepthStencil.depth,
                                          resource->info.texture.clearDepthStencil.value);
                } else {
                    cb->addImageBarrier(
                        resource->info.texture.handle, ResourceState::Undefined, ResourceState::RenderTarget, 0, 1);

                    const auto& clear = resource->info.texture.clearColor;
                    cb->clearColor(i, clear.red, clear.green, clear.blue, clear.alpha);
                }
            }
        }

        auto renderPass  = node->renderPass;
        auto framebuffer = node->framebuffer;

        if (!node->outputCount) {
            width       = _device->getSwapchainExtent().width;
            height      = _device->getSwapchainExtent().height;
            renderPass  = _device->getSwapchainPass();
            framebuffer = _device->getSwapchainFramebuffer();
        }

        cb->setFramebufferHeight(height);
        cb->setViewport(0, 0, width, height, 0.0f, 1.0f);
        cb->setScissor(0, 0, width, height);
        cb->setFrameGraph(&frameGraph);
        cb->bindPass(renderPass, framebuffer);

        if (!pass->pass.render(alias_cast<gerium_frame_graph_t>(&frameGraph), this, cb, pass->data)) {
            error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
        }

        if (node->outputCount) {
            cb->endCurrentRenderPass();
        }

        cb->popMarker();
    }

    cb->popMarker();

    cb->pushMarker("imgui");
    cb->bindPass(_device->getSwapchainPass(), _device->getSwapchainFramebuffer());
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb->vkCommandBuffer());
    cb->endCurrentRenderPass();
    cb->popMarker();

    cb->popMarker();
    _device->submit(cb);
}

void VkRenderer::onPresent() {
    _device->present();
}

Profiler* VkRenderer::onGetProfiler() noexcept {
    return _device->profiler();
}

void VkRenderer::onGetSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept {
    const auto& size = _device->getSwapchainExtent();
    width            = size.width;
    height           = size.height;
}

} // namespace gerium::vulkan
