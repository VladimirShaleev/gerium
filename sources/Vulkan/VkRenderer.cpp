#include "VkRenderer.hpp"
#include "../FrameGraph.hpp"

namespace gerium::vulkan {

VkRenderer::VkRenderer(Application* application, ObjectPtr<Device>&& device) noexcept :
    _application(application),
    _device(std::move(device)),
    _currentRenderPass(nullptr) {
    if (!application->isRunning()) {
        error(GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING);
    }
}

PipelineHandle VkRenderer::getPipeline(MaterialHandle handle) const noexcept {
    auto material = _materials.access(handle);

    auto it = std::lower_bound(material->passes,
                               material->passes + material->passCount,
                               _currentRenderPass,
                               [](const auto& p1, const auto& pass) {
        return p1.render_pass < pass;
    });

    return it != material->passes + material->passCount ? it->pipeline : Undefined;
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

MaterialHandle VkRenderer::onCreateMaterial(const FrameGraph& frameGraph,
                                            gerium_utf8_t name,
                                            gerium_uint32_t pipelineCount,
                                            const gerium_pipeline_t* pipelines) {
    auto [handle, material] = _materials.obtain_and_access();
    material->name          = intern(name);

    ViewportState viewport{};

    for (gerium_uint32_t i = 0; i < pipelineCount; ++i) {
        PipelineCreation pc{};
        pc.rasterization.cullMode = VK_CULL_MODE_NONE;
        pc.rasterization.front    = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pc.rasterization.fill     = FillMode::Solid;
        // pc.depthStencil.front            = ;
        // pc.depthStencil.back             = ;
        pc.depthStencil.depthComparison  = VK_COMPARE_OP_ALWAYS;
        pc.depthStencil.depthEnable      = 0;
        pc.depthStencil.depthWriteEnable = 0;
        pc.depthStencil.stencilEnable    = 0;
        // pc.program                       = ;
        pc.viewport = &viewport;
        pc.name     = pipelines[i].render_pass;

        for (gerium_uint32_t j = 0; j < pipelines[i].shader_count; ++j) {
            const auto& shader = pipelines[i].shaders[j];
            pc.program.addStage(shader.data,
                                shader.size,
                                shader.type == GERIUM_SHADER_TYPE_VERTEX ? VK_SHADER_STAGE_VERTEX_BIT
                                                                         : VK_SHADER_STAGE_FRAGMENT_BIT);
            pc.program.setName("simple");
        }

        for (gerium_uint32_t j = 0; j < pipelines[i].vertex_attribute_count; ++j) {
            const auto& attribute = pipelines[i].vertex_attributes[j];
            VertexAttribute a{};
            a.location = attribute.location;
            a.binding  = attribute.binding;
            a.offset   = attribute.offset;
            switch (attribute.format) {
                case GERIUM_FORMAT_R32G32_SFLOAT:
                    a.format = VertexComponentFormat::Float2;
                    break;

                case GERIUM_FORMAT_R32G32B32_SFLOAT:
                    a.format = VertexComponentFormat::Float3;
                    break;

                default:
                    break;
            }
            pc.vertexInput.addVertexAttribute(a);
        }

        for (gerium_uint32_t j = 0; j < pipelines[i].vertex_binding_count; ++j) {
            const auto& binding = pipelines[i].vertex_bindings[j];
            VertexStream s{};
            s.binding = binding.binding;
            s.stride  = binding.stride;
            switch (binding.inputRate) {
                case GERIUM_VERTEX_RATE_PER_VERTEX:
                    s.inputRate = VertexInputRate::PerVertex;
                    break;

                case GERIUM_VERTEX_RATE_PER_INSTANCE:
                    s.inputRate = VertexInputRate::PerInstance;
                    break;

                default:
                    break;
            }
            pc.vertexInput.addVertexStream(s);
        }

        if (auto node = frameGraph.getNode(pipelines[i].render_pass); node && node->renderPass != Undefined) {
            pc.renderPass = _device->getRenderPassOutput(node->renderPass);
        } else {
            pc.renderPass = _device->getRenderPassOutput(_device->getSwapchainPass());
        }

        material->passes[i].render_pass = intern(pipelines[i].render_pass);
        material->passes[i].pipeline    = _device->createPipeline(pc);
        ++material->passCount;
    }

    std::sort(material->passes, material->passes + material->passCount, [](const auto& mat1, const auto& mat2) {
        return mat1.render_pass < mat2.render_pass;
    });

    return handle;
}

DescriptorSetHandle VkRenderer::onCreateDescriptorSet() {
    return DescriptorSetHandle();
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
                creation.output.setDepthStencilOperations(info.texture.operation, GERIUM_RENDER_PASS_OPERATION_LOAD);
            } else {
                creation.output.color(
                    format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, GERIUM_RENDER_PASS_OPERATION_LOAD);
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

void VkRenderer::onDestroyMaterial(MaterialHandle handle) noexcept {
    auto meterial = _materials.access(handle);

    for (gerium_uint32_t i = 0; i < meterial->passCount; ++i) {
        _device->destroyPipeline(meterial->passes[i].pipeline);
    }

    _materials.release(handle);
}

void VkRenderer::onDestroyDescriptorSet(DescriptorSetHandle handle) noexcept {
    _device->destroyDescriptorSet(handle);
}

void VkRenderer::onDestroyRenderPass(RenderPassHandle handle) noexcept {
    _device->destroyRenderPass(handle);
}

void VkRenderer::onDestroyFramebuffer(FramebufferHandle handle) noexcept {
    _device->destroyFramebuffer(handle);
}

bool VkRenderer::onNewFrame() {
    return _device->newFrame();
}

void VkRenderer::onRender(const FrameGraph& frameGraph) {
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

        // TODO: remove
        cb->clearColor(1.0f, 0.0f, 1.0f, 1.0f);
        cb->clearDepthStencil(1.0f, 0.0f);

        gerium_uint16_t width;
        gerium_uint16_t height;

        for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
            auto resource = frameGraph.getResource(node->inputs[i]);

            if (resource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE) {
                cb->addImageBarrier(resource->info.texture.handle,
                                    ResourceState::RenderTarget,
                                    ResourceState::ShaderResource,
                                    0,
                                    1,
                                    hasDepth(toVkFormat(resource->info.texture.format)),
                                    hasStencil(toVkFormat(resource->info.texture.format)));
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

                if (hasDepthOrStencil(toVkFormat(resource->info.texture.format))) {
                    cb->addImageBarrier(resource->info.texture.handle,
                                        ResourceState::Undefined,
                                        ResourceState::DepthWrite,
                                        0,
                                        1,
                                        true,
                                        hasStencil(toVkFormat(resource->info.texture.format)));
                } else {
                    cb->addImageBarrier(resource->info.texture.handle,
                                        ResourceState::Undefined,
                                        ResourceState::RenderTarget,
                                        0,
                                        1,
                                        false,
                                        false);
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

        Rect2DInt scissor{ 0, 0, width, height };
        Viewport viewport{};
        viewport.rect      = { 0, 0, width, height };
        viewport.min_depth = 0.0f;
        viewport.max_depth = 1.0f;

        cb->setScissor(&scissor);
        cb->setViewport(&viewport);

        if (pass->pass.prepare) {
            pass->pass.prepare(alias_cast<gerium_frame_graph_t>(&frameGraph), this, pass->data);
        }

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
    _device->drawProfiler();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cb->vkCommandBuffer());
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

} // namespace gerium::vulkan
