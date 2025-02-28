#include "VkRenderer.hpp"
#include "../FrameGraph.hpp"

namespace gerium::vulkan {

VkRenderer::VkRenderer(Application* application, ObjectPtr<Device>&& device) noexcept :
    _application(application),
    _device(std::move(device)),
    _isSupportedTransferQueue(false),
    _width(0),
    _height(0),
    _currentRenderPassName(nullptr),
    _transferMaxTasks(10),
    _transferBuffer(Undefined),
    _transferBufferOffset(0),
    _loadEvent(marl::Event::Mode::Manual),
    _loadThreadEnd(marl::Event::Mode::Manual),
    _prevFrame(1),
    _frame(0) {
    if (!application->isRunning()) {
        error(GERIUM_RESULT_ERROR_APPLICATION_NOT_RUNNING);
    }
}

VkRenderer::~VkRenderer() {
    closeLoadThread();
    _loadEvent.signal();
    _loadThreadEnd.signal();
    if (_isSupportedTransferQueue) {
        _loadTread.join();
    }
}

PipelineHandle VkRenderer::getPipeline(TechniqueHandle handle) const noexcept {
    auto technique = _techniques.access(handle);

    auto it = std::lower_bound(technique->passes,
                               technique->passes + technique->passCount,
                               _currentRenderPassName,
                               [](const auto& p1, const auto& pass) {
        return p1.render_pass < pass;
    });

    return it != technique->passes + technique->passCount ? it->pipeline : Undefined;
}

void VkRenderer::onInitialize(gerium_feature_flags_t features, gerium_uint32_t version, bool debug) {
    _device->create(application(), features, version, debug);
    _isSupportedTransferQueue = _device->isSupportedTransferQueue();
    createTransferBuffer();
}

Application* VkRenderer::application() noexcept {
    return _application.get();
}

void VkRenderer::createTransferBuffer() {
    BufferCreation bc;
    bc.reset().set({}, ResourceUsageType::Staging, 256 * 1024 * 1024).setName("staging_buffer").setPersistent(true);
    _transferBuffer       = _device->createBuffer(bc);
    _transferBufferOffset = 0;

    _transferCommandPool.create(*_device.get(), 1, _transferMaxTasks, QueueType::CopyTransfer);

    if (_isSupportedTransferQueue) {
        auto scheduler = marl::Scheduler::get();

        _loadTread = std::thread([this, scheduler]() {
            scheduler->bind();
            defer(scheduler->unbind());
            loadThread();
        });
    }
}

void VkRenderer::sendTextureToGraphic() {
    if (!_isSupportedTransferQueue) {
        if (!_loadRequests.empty()) {
            auto request = _loadRequests.front();
            _loadRequests.pop();

            gerium_texture_info_t info;
            onGetTextureInfo(request.texture, info);

            const auto blockSize        = vk::blockSize((vk::Format) toVkFormat(info.format));
            const auto alignedImageSize = align(info.width * info.height * blockSize, 4);

            auto data = onMapBuffer(_transferBuffer, 0, alignedImageSize);
            memcpy((void*) data, request.data, alignedImageSize);
            onUnmapBuffer(_transferBuffer);

            auto commandBuffer = _transferCommandPool.getPrimary(0, false);
            commandBuffer->copyBuffer(_transferBuffer, request.texture, request.mip);
            commandBuffer->submit(QueueType::CopyTransfer);

            _transferToGraphic.push(request);
        }
    }

    if (_isSupportedTransferQueue) {
        _transferToGraphicMutex.lock();
    }
    defer(if (_isSupportedTransferQueue) _transferToGraphicMutex.unlock());
    if (!_transferToGraphic.empty()) {
        auto commandBuffer = _device->getPrimaryCommandBuffer(false);
        while (!_transferToGraphic.empty()) {
            const auto& request = _transferToGraphic.front();
            if (request.generateMips) {
                commandBuffer->generateMipmaps(request.texture);
                _device->finishLoadTexture(request.texture, 0);
            } else {
                commandBuffer->addImageBarrier(request.texture, ResourceState::ShaderResource, request.mip, 1);
                _device->finishLoadTexture(request.texture, request.mip);
                _device->showViewMips(request.texture, request.mip);
            }
            _finishedRequests.push(request);
            _transferToGraphic.pop();
        }
        _device->submit(commandBuffer);
    }
}

bool VkRenderer::isResourceEnabled(FrameGraph& frameGraph, const FrameGraphResource* resource) const noexcept {
    if (resource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE || resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
        auto index = 0;
        if (resource->info.texture.handles[1] != Undefined) {
            index = resource->saveForNextFrame ? _prevFrame : _frame;
        }
        if (resource->info.texture.handles[index] != Undefined) {
            return true;
        }

    } else if (resource->info.type == GERIUM_RESOURCE_TYPE_BUFFER) {
        if (resource->info.buffer.handle != Undefined) {
            return true;
        }
    } else {
        return true;
    }
    return frameGraph.getNode(resource->producer)->enabled;
}

gerium_feature_flags_t VkRenderer::onGetEnabledFeatures() const noexcept {
    auto result = GERIUM_FEATURE_NONE_BIT;
    if (_device->bindlessSupported()) {
        result |= GERIUM_FEATURE_BINDLESS_BIT;
    }
    if (_device->meshShaderSupported()) {
        result |= GERIUM_FEATURE_MESH_SHADER_BIT;
    }
    if (_device->samplerFilterMinmaxSupported()) {
        result |= GERIUM_FEATURE_SAMPLER_FILTER_MINMAX_BIT;
    }
    if (_device->is8BitStorageSupported()) {
        result |= GERIUM_FEATURE_8_BIT_STORAGE_BIT;
    }
    if (_device->is16BitStorageSupported()) {
        result |= GERIUM_FEATURE_16_BIT_STORAGE_BIT;
    }
    return result;
}

TextureCompressionFlags VkRenderer::onGetTextureComperssion() const noexcept {
    return _device->compressions();
}

bool VkRenderer::onGetProfilerEnable() const noexcept {
    return _device->isProfilerEnable();
}

void VkRenderer::onSetProfilerEnable(bool enable) noexcept {
    _device->setProfilerEnable(enable);
}

bool VkRenderer::onIsSupportedFormat(gerium_format_t format) noexcept {
    return _device->isSupportedFormat(format);
}

void VkRenderer::onGetTextureInfo(TextureHandle handle, gerium_texture_info_t& info) noexcept {
    _device->getTextureInfo(handle, info);
}

BufferHandle VkRenderer::onCreateBuffer(const BufferCreation& creation) {
    return _device->createBuffer(creation);
}

TextureHandle VkRenderer::onCreateTexture(const TextureCreation& creation) {
    return _device->createTexture(creation);
}

TextureHandle VkRenderer::onCreateTextureView(const TextureViewCreation& creation) {
    return _device->createTextureView(creation);
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

DescriptorSetHandle VkRenderer::onCreateDescriptorSet(bool global) {
    DescriptorSetCreation creation{};
    creation.setGlobal(global);
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
                creation.output.setDepthStencilOperations(RenderPassOp::Load, RenderPassOp::Load);
            } else {
                creation.output.color(format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, RenderPassOp::Load);
            }
        }
    }

    return _device->createRenderPass(creation);
}

FramebufferHandle VkRenderer::onCreateFramebuffer(const FrameGraph& frameGraph,
                                                  const FrameGraphNode* node,
                                                  gerium_uint32_t textureIndex) {
    FramebufferCreation creation{};
    creation.renderPass = node->renderPass;
    creation.setName(node->name);

    gerium_uint16_t width  = 0;
    gerium_uint16_t height = 0;
    gerium_uint16_t layers = 1;

    for (gerium_uint32_t i = 0; i < node->outputCount; ++i) {
        auto resource = frameGraph.getResource(node->outputs[i]);
        auto& info    = resource->info;
        auto index    = resource->saveForNextFrame ? textureIndex : 0;

        if (info.type != GERIUM_RESOURCE_TYPE_ATTACHMENT) {
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

        layers = std::max(layers, info.texture.layers);

        if (hasDepthOrStencil(toVkFormat(info.texture.format))) {
            creation.setDepthStencilTexture(info.texture.handles[index]);
        } else {
            creation.addRenderTexture(info.texture.handles[index]);
        }
    }

    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto inputResource = frameGraph.getResource(node->inputs[i]);

        if (inputResource->info.type != GERIUM_RESOURCE_TYPE_ATTACHMENT) {
            continue;
        }

        auto resource = frameGraph.getResource(inputResource->name);
        auto& info    = resource->info;
        auto index    = resource->saveForNextFrame ? textureIndex : 0;

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

        layers = std::max(layers, info.texture.layers);

        if (hasDepthOrStencil(toVkFormat(info.texture.format))) {
            creation.setDepthStencilTexture(info.texture.handles[index]);
        } else {
            creation.addRenderTexture(info.texture.handles[index]);
        }
    }

    creation.width  = width;
    creation.height = height;
    creation.layers = layers;

    return _device->createFramebuffer(creation);
}

void VkRenderer::onAsyncUploadTextureData(TextureHandle handle,
                                          gerium_uint8_t mip,
                                          bool generateMips,
                                          gerium_uint32_t textureDataSize,
                                          gerium_cdata_t textureData,
                                          gerium_texture_loaded_func_t callback,
                                          gerium_data_t data) {
    const auto request = LoadRequest{ textureDataSize, textureData, mip, generateMips, handle, callback, data };
    if (_isSupportedTransferQueue) {
        marl::lock lock(_loadRequestsMutex);
        _loadRequests.push(request);
        _loadEvent.signal();
    } else {
        _loadRequests.push(request);
    }
}

void VkRenderer::onTextureSampler(TextureHandle handle,
                                  gerium_filter_t minFilter,
                                  gerium_filter_t magFilter,
                                  gerium_filter_t mipFilter,
                                  gerium_address_mode_t addressModeU,
                                  gerium_address_mode_t addressModeV,
                                  gerium_address_mode_t addressModeW,
                                  gerium_reduction_mode_t reductionMode) {
    SamplerCreation sc;
    sc.setMinMagMip(toVkFilter(minFilter), toVkFilter(magFilter), toVkSamplerMipmapMode(mipFilter))
        .setAddressModeUvw(toVkSamplerAddressMode(addressModeU),
                           toVkSamplerAddressMode(addressModeV),
                           toVkSamplerAddressMode(addressModeW))
        .setReductionMode(toVkSamplerReductionMode(reductionMode));
    auto oldSampler = _device->getTextureSampler(handle);
    auto sampler    = _device->createSampler(sc);
    _device->linkTextureSampler(handle, sampler);
    if (oldSampler != Undefined) {
        _device->destroySampler(oldSampler);
    }
}

BufferHandle VkRenderer::onGetBuffer(gerium_utf8_t resource) {
    if (auto handle = _device->findInputResource(resource, false); handle != Undefined) {
        return handle;
    }
    throw Exception(GERIUM_RESULT_ERROR_INVALID_ARGUMENT);
}

TextureHandle VkRenderer::onGetTexture(gerium_utf8_t resource, bool fromPreviousFrame) {
    if (auto handle = _device->findInputResource(resource, fromPreviousFrame); handle != Undefined) {
        return handle;
    }
    throw Exception(GERIUM_RESULT_ERROR_INVALID_ARGUMENT);
}

void VkRenderer::onDestroyBuffer(BufferHandle handle) noexcept {
    _device->destroyBuffer(handle);
}

void VkRenderer::onDestroyTexture(TextureHandle handle) noexcept {
    _device->destroyTexture(handle);
}

void VkRenderer::onDestroyTechnique(TechniqueHandle handle) noexcept {
    if (_techniques.references(handle) == 1) {
        auto technique = _techniques.access(handle);

        for (gerium_uint32_t i = 0; i < technique->passCount; ++i) {
            _device->destroyPipeline(technique->passes[i].pipeline);
        }
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
    _device->bind(handle, binding, 0, buffer, _device->isBufferDynamic(buffer));
}

void VkRenderer::onBind(DescriptorSetHandle handle,
                        gerium_uint16_t binding,
                        gerium_uint16_t element,
                        TextureHandle texture) noexcept {
    _device->bind(handle, binding, element, texture);
}

void VkRenderer::onBind(DescriptorSetHandle handle,
                        gerium_uint16_t binding,
                        gerium_utf8_t resourceInput,
                        bool fromPreviousFrame) noexcept {
    _device->bind(handle, binding, 0, Undefined, false, resourceInput, fromPreviousFrame);
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
    if (!_device->newFrame()) {
        return false;
    }

    sendTextureToGraphic();
    return true;
}

void VkRenderer::onRender(FrameGraph& frameGraph) {
    const auto maxWorkers = _application->workerThreadCount();

    gerium_uint16_t width, height;
    getSwapchainSize(width, height);

    if (_width != width || _height != height) {
        if (_width != 0 && _height != 0) {
            frameGraph.resize(_width, width, _height, height);
        }
        _width  = width;
        _height = height;
    }
    frameGraph.compile();

    gerium_uint32_t allTotalWorkers[kMaxNodes];
    for (gerium_uint32_t i = 0, worker = 0; i < frameGraph.nodeCount(); ++i) {
        auto node = frameGraph.getNode(i);

        if (!node->enabled) {
            continue;
        }

        allTotalWorkers[worker] = 1;
        if (auto pass = frameGraph.getPass(node->pass); pass->pass.prepare) {
            allTotalWorkers[worker] = std::clamp(
                pass->pass.prepare(alias_cast<gerium_frame_graph_t>(&frameGraph), this, maxWorkers, pass->data),
                (gerium_uint32_t) 1,
                maxWorkers);
            if (allTotalWorkers[worker] == 0) {
                error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
            }
        }
        ++worker;
    }

    CommandBuffer* secondaryCommandBuffers[100];
    gerium_uint32_t numSecondaryCommandBuffers = 0;

    std::set<TextureHandle> depths;

    auto cb = _device->getPrimaryCommandBuffer();
    cb->bindRenderer(this);
    cb->pushMarker("total");

    cb->pushMarker("frame_graph");
    for (gerium_uint32_t i = 0, totalWorkerIndex = 0; i < frameGraph.nodeCount(); ++i) {
        auto node = frameGraph.getNode(i);
        if (!node->enabled) {
            continue;
        }
        _currentRenderPassName = node->name;

        cb->pushLabel(node->name);
        cb->pushMarker(node->name);

        gerium_uint16_t width;
        gerium_uint16_t height;

        _device->clearInputResources();
        for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
            frameGraph.fillExternalResource(node->inputs[i]);
            auto resource = frameGraph.getResource(node->inputs[i]);
            if (!isResourceEnabled(frameGraph, resource)) {
                continue;
            }

            if (resource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE) {
                auto index = 0;
                if (resource->info.texture.handles[1] != Undefined) {
                    index = resource->saveForNextFrame ? _prevFrame : _frame;
                }

                auto texture      = resource->info.texture.handles[index];
                const auto format = toVkFormat(resource->info.texture.format);

                if (hasDepthOrStencil(format)) {
                    cb->addImageBarrier(texture, ResourceState::DepthRead, 0, 1);
                    depths.insert(texture);
                } else {
                    cb->addImageBarrier(texture, ResourceState::ShaderResource, 0, 1);
                }
                _device->addInputResource(resource, texture, resource->saveForNextFrame);
            } else if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
                width  = resource->info.texture.width;
                height = resource->info.texture.height;

                auto index = 0;
                if (resource->info.texture.handles[1] != Undefined) {
                    index = resource->saveForNextFrame ? _prevFrame : _frame;
                }
                auto texture      = resource->info.texture.handles[index];
                const auto format = toVkFormat(resource->info.texture.format);

                if (hasDepthOrStencil(format)) {
                    cb->addImageBarrier(
                        texture, !node->compute ? ResourceState::DepthWrite : ResourceState::UnorderedAccess, 0, 1);
                    depths.insert(texture);
                } else {
                    cb->addImageBarrier(
                        texture, !node->compute ? ResourceState::RenderTarget : ResourceState::UnorderedAccess, 0, 1);
                }
                if (node->compute) {
                    _device->addInputResource(resource, texture, resource->saveForNextFrame);
                }
            } else if (resource->info.type == GERIUM_RESOURCE_TYPE_BUFFER) {
                auto buffer = resource->info.buffer.handle;
                constexpr auto states =
                    ResourceState::UnorderedAccess | ResourceState::IndirectArgument | ResourceState::ShaderResource;
                cb->addBufferBarrier(buffer, states);
                _device->addInputResource(resource, buffer, false);
            }
        }

        for (gerium_uint32_t i = 0; i < node->outputCount; ++i) {
            frameGraph.fillExternalResource(node->outputs[i]);
            auto resource = frameGraph.getResource(node->outputs[i]);

            if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
                auto index = resource->saveForNextFrame ? _frame : 0;
                width      = resource->info.texture.width;
                height     = resource->info.texture.height;

                const auto format  = toVkFormat(resource->info.texture.format);
                const auto texture = resource->info.texture.handles[index];

                if (hasDepthOrStencil(format)) {
                    cb->addImageBarrier(
                        texture, !node->compute ? ResourceState::DepthWrite : ResourceState::UnorderedAccess, 0, 1);
                    depths.insert(texture);
                    if (!node->compute) {
                        cb->clearDepthStencil(resource->info.texture.clearDepthStencil.depth,
                                              resource->info.texture.clearDepthStencil.value);
                    }
                } else {
                    cb->addImageBarrier(
                        texture, !node->compute ? ResourceState::RenderTarget : ResourceState::UnorderedAccess, 0, 1);
                    if (!node->compute) {
                        const auto& clear = resource->info.texture.clearColor;
                        cb->clearColor(i, clear.red, clear.green, clear.blue, clear.alpha);
                    }
                }
                if (node->compute) {
                    _device->addInputResource(resource, texture, false);
                }
                _device->finishLoadTexture(texture, 0, true);
            } else if (resource->info.type == GERIUM_RESOURCE_TYPE_BUFFER) {
                auto buffer = resource->info.buffer.handle;
                cb->addBufferBarrier(buffer, ResourceState::UnorderedAccess);
                _device->addInputResource(resource, buffer, false);
            }
        }

        auto framebufferIndex = node->framebuffers[1] != Undefined ? _frame : 0;

        auto pass         = frameGraph.getPass(node->pass);
        auto totalWorkers = allTotalWorkers[totalWorkerIndex];
        auto renderPass   = node->renderPass;
        auto framebuffer  = node->framebuffers[framebufferIndex];
        auto useWorkers   = totalWorkers != 1;

        if (!node->outputCount) {
            width       = _device->getSwapchainExtent().width;
            height      = _device->getSwapchainExtent().height;
            renderPass  = _device->getSwapchainPass();
            framebuffer = _device->getSwapchainFramebuffer();
        }

        cb->setFrameGraph(&frameGraph);
        if (!node->compute) {
            cb->setFramebufferHeight(height);
            cb->setViewport(0, 0, width, height, 0.0f, 1.0f);
            cb->setScissor(0, 0, width, height);
            cb->bindPass(renderPass, framebuffer, useWorkers);
            if (useWorkers) {
                marl::WaitGroup waitAll(totalWorkers);
                numSecondaryCommandBuffers = 0;
                for (gerium_uint32_t worker = 0; worker < totalWorkers; ++worker) {
                    auto secondary = _device->getSecondaryCommandBuffer(worker, renderPass, framebuffer);
                    secondary->bindRenderer(this);
                    secondary->setFramebufferHeight(height);
                    secondary->setViewport(0, 0, width, height, 0.0f, 1.0f);
                    secondary->setScissor(0, 0, width, height);
                    secondary->setFrameGraph(&frameGraph);
                    secondaryCommandBuffers[numSecondaryCommandBuffers++] = secondary;

                    marl::schedule([waitAll, worker, totalWorkers, cb = secondary, pass, renderer = this, &frameGraph] {
                        defer(waitAll.done());

                        if (!pass->pass.render(alias_cast<gerium_frame_graph_t>(&frameGraph),
                                               renderer,
                                               cb,
                                               worker,
                                               totalWorkers,
                                               pass->data)) {
                            error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
                        }
                    });
                }

                waitAll.wait();

                cb->execute(numSecondaryCommandBuffers, secondaryCommandBuffers);
            } else {
                if (!pass->pass.render(alias_cast<gerium_frame_graph_t>(&frameGraph), this, cb, 0, 1, pass->data)) {
                    error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
                }
            }

            if (!node->outputCount) {
                auto imguiCb = useWorkers ? _device->getSecondaryCommandBuffer(
                                                0, _device->getSwapchainPass(), _device->getSwapchainFramebuffer())
                                          : cb;
                imguiCb->bindRenderer(this);
                imguiCb->setFramebufferHeight(height);
                imguiCb->setViewport(0, 0, width, height, 0.0f, 1.0f);
                imguiCb->setScissor(0, 0, width, height);
                imguiCb->setFrameGraph(&frameGraph);

                ImGui::Render();
                imguiCb->pushLabel("imgui");
                imguiCb->pushMarker("imgui");
                ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCb->vkCommandBuffer());
                imguiCb->popMarker();
                imguiCb->popLabel();
                if (useWorkers) {
                    cb->execute(1, &imguiCb);
                }
            }

            cb->endCurrentRenderPass();
        } else {
            if (!pass->pass.render(alias_cast<gerium_frame_graph_t>(&frameGraph), this, cb, 0, 1, pass->data)) {
                error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
            }
        }

        cb->popMarker();
        cb->popLabel();

        ++totalWorkerIndex;
    }
    cb->popMarker();

    for (auto depth : depths) {
        cb->addImageBarrier(depth, ResourceState::DepthRead, 0, 1);
    }

    cb->popMarker();
    _device->submit(cb);
}

void VkRenderer::onPresent() {
    _device->present();

    while (!_finishedRequests.empty()) {
        const auto& request = _finishedRequests.front();
        if (request.callback) {
            request.callback(this, request.texture, request.userData);
        }
        _finishedRequests.pop();
    }

    _prevFrame = _frame;
    _frame     = (_frame + 1) % 2;
}

FfxInterface VkRenderer::onCreateFfxInterface(gerium_uint32_t maxContexts) {
    return _device->createFfxInterface(maxContexts);
}

void VkRenderer::onWaitFfxJobs() const noexcept {
    _device->waitFfxJobs();
}

void VkRenderer::onDestroyFfxInterface(FfxInterface* ffxInterface) noexcept {
    _device->destroyFfxInterface(ffxInterface);
}

FfxResource VkRenderer::onGetFfxBuffer(BufferHandle handle) const noexcept {
    return _device->ffxBuffer(handle);
}

FfxResource VkRenderer::onGetFfxTexture(TextureHandle handle) const noexcept {
    return _device->ffxTexture(handle);
}

Profiler* VkRenderer::onGetProfiler() noexcept {
    return _device->profiler();
}

void VkRenderer::onGetSwapchainSize(gerium_uint16_t& width, gerium_uint16_t& height) const noexcept {
    const auto& size = _device->getSwapchainExtent();
    width            = size.width;
    height           = size.height;
}

void VkRenderer::loadThread() noexcept {
    LoadRequest request;
    std::vector<LoadRequest> tasks;

    auto finishLoad = [this, &tasks]() {
        marl::lock lock(_transferToGraphicMutex);
        for (auto& task : tasks) {
            _transferToGraphic.push(task);
        }
        tasks.clear();
    };

    while (!_loadThreadEnd.test()) {
        _loadEvent.wait();
        {
            marl::lock lock(_loadRequestsMutex);
            if (_loadRequests.empty()) {
                _loadEvent.clear();
                _transferCommandPool.wait(QueueType::CopyTransfer);
                _transferBufferOffset = 0;
                finishLoad();
                continue;
            }
            request = _loadRequests.front();
            _loadRequests.pop();

            tasks.push_back(request);
        }

        gerium_texture_info_t info;
        onGetTextureInfo(request.texture, info);

        const auto blockSize = vk::blockSize((vk::Format) toVkFormat(info.format));
        const auto alignedImageSize =
            request.dataSize ? request.dataSize : align(info.width * info.height * blockSize, 4);
        const auto currentOffset = _transferBufferOffset;
        _transferBufferOffset += alignedImageSize;

        auto data = onMapBuffer(_transferBuffer, (gerium_uint32_t) currentOffset, alignedImageSize);
        memcpy((void*) data, request.data, alignedImageSize);
        onUnmapBuffer(_transferBuffer);

        auto commandBuffer = _transferCommandPool.getPrimary(0, false);
        commandBuffer->copyBuffer(_transferBuffer, request.texture, request.mip, (gerium_uint32_t) currentOffset);
        commandBuffer->submit(QueueType::CopyTransfer, false);

        if (tasks.size() >= _transferMaxTasks) {
            _transferCommandPool.wait(QueueType::CopyTransfer);
            _transferBufferOffset = 0;
            finishLoad();
        }
    }
}

} // namespace gerium::vulkan
