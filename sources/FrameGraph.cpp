#include "FrameGraph.hpp"
#include "StringPool.hpp"

namespace gerium {

FrameGraph::~FrameGraph() {
    clear();
}

FrameGraph::FrameGraph(Renderer* renderer) :
    _logger(Logger::create("gerium:frame-graph")),
    _renderer(renderer),
    _hasChanges(false),
    _nodeGraphCount(0) {
}

void FrameGraph::addPass(gerium_utf8_t name, const gerium_render_pass_t* renderPass, gerium_data_t data) {
    const auto key = hash(name);

    if (_renderPassCache.contains(key)) {
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name](auto& stream) {
            stream << "Render pass '" << name << "' already exists in frame graph";
        });
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err GERIUM_RESULT_ERROR_EXISTS;
    }

    auto [handle, pass] = _renderPasses.obtain_and_access();

    pass->pass = *renderPass;
    pass->name = intern(name);
    pass->data = data;

    _renderPassCache.insert({ key, handle });

    _hasChanges = true;
}

void FrameGraph::removePass(gerium_utf8_t name) {
    const auto key = hash(name);

    if (auto it = _renderPassCache.find(key); it != _renderPassCache.end()) {
        _renderPassCache.erase(it);
    } else {
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name](auto& stream) {
            stream << "Render pass '" << name << "' not found";
        });
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err GERIUM_RESULT_ERROR_NOT_FOUND;
    }

    _hasChanges = true;
}

void FrameGraph::addNode(gerium_utf8_t name,
                         bool compute,
                         gerium_uint32_t inputCount,
                         const gerium_resource_input_t* inputs,
                         gerium_uint32_t outputCount,
                         const gerium_resource_output_t* outputs) {
    const auto key = hash(name);

    if (_nodeCache.contains(key)) {
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name](auto& stream) {
            stream << "Node '" << name << "' already exists in frame graph";
        });
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err GERIUM_RESULT_ERROR_EXISTS;
    }

    auto [handle, node] = _nodes.obtain_and_access();

    node->renderPass      = Undefined;
    node->framebuffers[0] = Undefined;
    node->framebuffers[1] = Undefined;
    node->pass            = Undefined;
    node->name            = intern(name);
    node->compute         = compute;
    node->enabled         = 1;

    for (gerium_uint32_t i = 0; i < inputCount; ++i) {
        node->inputs[node->inputCount++] = createNodeInput(inputs[i]);
    }

    for (gerium_uint32_t i = 0; i < outputCount; ++i) {
        node->outputs[node->outputCount++] = createNodeOutput(outputs[i], handle);
    }

    _nodeGraph[_nodeGraphCount++] = handle;
    _nodeCache.insert({ key, handle });

    _hasChanges = true;
}

void FrameGraph::enableNode(gerium_utf8_t name, gerium_bool_t enable) {
    const auto key = hash(name);

    if (auto it = _nodeCache.find(key); it != _nodeCache.end()) {
        auto node = _nodes.access(it->second);
        if ((node->enabled != 0) != (enable != 0)) {
            node->enabled = enable;
            _hasChanges   = true;
        }
    } else {
        _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name](auto& stream) {
            stream << "Node '" << name << "' not found in frame graph";
        });
        error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err GERIUM_RESULT_ERROR_NOT_FOUND;
    }
}

void FrameGraph::addBuffer(gerium_utf8_t name, BufferHandle handle) {
    if (handle != Undefined) {
        _externalCache[hash(name)] = { handle, false };
    } else {
        _externalCache.erase(hash(name));
    }
}

void FrameGraph::addTexture(gerium_utf8_t name, TextureHandle handle) {
    if (handle != Undefined) {
        _externalCache[hash(name)] = { handle, true };
    } else {
        _externalCache.erase(hash(name));
    }
}

void FrameGraph::clear() {
    _hasChanges = false;

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);

        for (auto& framebuffer : node->framebuffers) {
            if (framebuffer != Undefined) {
                _renderer->destroyFramebuffer(framebuffer);
                framebuffer = Undefined;
            }
        }

        if (node->renderPass != Undefined) {
            _renderer->destroyRenderPass(node->renderPass);
        }

        for (gerium_uint32_t j = 0; j < node->outputCount; ++j) {
            auto resource = _resources.access(node->outputs[j]);

            if (resource->external) {
                continue;
            }

            if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
                for (auto& handle : resource->info.texture.handles) {
                    if (handle != Undefined) {
                        _renderer->destroyTexture(handle);
                        handle = Undefined;
                    }
                }
            } else if (resource->info.type == GERIUM_RESOURCE_TYPE_BUFFER) {
                if (resource->info.buffer.handle != Undefined) {
                    _renderer->destroyBuffer(resource->info.buffer.handle);
                    resource->info.buffer.handle = Undefined;
                }
            }
        }
    }
    _nodeGraphCount = 0;

    _renderPassCache.clear();
    _resourceCache.clear();
    _nodeCache.clear();

    _renderPasses.releaseAll();
    _resources.releaseAll();
    _nodes.releaseAll();
}

void FrameGraph::compile() {
    if (!_hasChanges) {
        return;
    }

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        _nodes.access(_nodeGraph[i])->edgeCount = 0;
    }

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);
        computeEdges(node);
    }

    gerium_uint32_t sortedNodeCount = 0;
    gerium_uint32_t stackCount      = 0;
    memset((void*) _visited.data(), 0, _visited.size());

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        if (!_nodes.access(_nodeGraph[i])->enabled) {
            continue;
        }

        _stack[stackCount++] = _nodeGraph[i];

        while (stackCount) {
            auto nodeHandle = _stack[stackCount - 1];

            if (_visited[nodeHandle.index] == 2) {
                --stackCount;
                continue;
            }

            if (_visited[nodeHandle.index] == 1) {
                _visited[nodeHandle.index]      = 2;
                _sortedNodes[sortedNodeCount++] = nodeHandle;
                --stackCount;
                continue;
            }

            _visited[nodeHandle.index] = 1;

            auto node = _nodes.access(nodeHandle);

            if (!node->edgeCount) {
                continue;
            }

            for (gerium_uint32_t e = 0; e < node->edgeCount; ++e) {
                auto childHandle = node->edges[e];

                if (!_visited[childHandle.index]) {
                    _stack[stackCount++] = childHandle;
                }
            }
        }
    }

    std::copy_n(_sortedNodes.crbegin() + (kMaxNodes - sortedNodeCount), sortedNodeCount, _nodeGraph.begin());

    for (auto& item : _allocations) {
        item = Undefined;
    }

    _freeList.clear();

    std::set<std::string> storedResources;

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);

        if (!node->enabled) {
            continue;
        }

        for (gerium_uint32_t j = 0; j < node->inputCount; ++j) {
            auto inputResource  = _resources.access(node->inputs[j]);
            auto outputResource = _resources.access(inputResource->output);

            if (inputResource->saveForNextFrame) {
                storedResources.insert(inputResource->name);
                outputResource->saveForNextFrame = inputResource->saveForNextFrame;
            }
            ++outputResource->refCount;
        }
    }

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);

        if (!node->enabled) {
            continue;
        }

        for (gerium_uint32_t j = 0; j < node->outputCount; ++j) {
            auto resourceIndex = node->outputs[j].index;
            auto resource      = _resources.access(node->outputs[j]);

            if (!resource->external && _allocations[resourceIndex] == Undefined) {
                _allocations[resourceIndex] = _nodeGraph[i];

                if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT) {
                    const auto& info = resource->info.texture;

                    TextureCreation creation{};
                    creation.setFormat(info.format, GERIUM_TEXTURE_TYPE_2D)
                        .setSize(info.width, info.height, info.depth)
                        .setFlags(1, 1, true, node->compute);

                    if (!_freeList.empty()) {
                        const auto size =
                            calcTextureSize(creation.width, creation.height, creation.depth, creation.format);

                        gerium_texture_info_t info;
                        auto it = _freeList.begin();
                        for (; it != _freeList.end(); ++it) {
                            _renderer->getTextureInfo(*it, info);
                            const auto freeSize = calcTextureSize(info.width, info.height, info.depth, info.format);
                            if (size <= freeSize && !storedResources.contains(resource->name)) {
                                break;
                            }
                        }
                        if (it != _freeList.end()) {
                            creation.setAlias(*it);
                            _freeList.erase(it);
                        }
                    }

                    int index = 0;
                    for (auto& handle : resource->info.texture.handles) {
                        std::string name;
                        if (resource->name) {
                            name = std::string(resource->name) + '-' + std::to_string(index++);
                            creation.setName(name.c_str());
                        }
                        if (handle == Undefined) {
                            handle = _renderer->createTexture(creation);
                        }
                        if (!resource->saveForNextFrame) {
                            break;
                        }
                    }
                } else if (resource->info.type == GERIUM_RESOURCE_TYPE_BUFFER) {
                    if (resource->info.buffer.handle == Undefined) {
                        BufferCreation creation{};
                        creation
                            .set(resource->info.buffer.usage, ResourceUsageType::Immutable, resource->info.buffer.size)
                            .setFillValue(resource->info.buffer.fillValue)
                            .setName(resource->name);
                        resource->info.buffer.handle = _renderer->createBuffer(creation);
                    }
                }
            }
        }

        for (gerium_uint32_t j = 0; j < node->inputCount; ++j) {
            auto inputResource = _resources.access(node->inputs[j]);

            auto resourceIndex = inputResource->output.index;
            auto resource      = _resources.access(inputResource->output);

            --resource->refCount;

            if (!resource->external && resource->refCount == 0 && !storedResources.contains(resource->name)) {
                if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT ||
                    resource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE) {
                    _freeList.insert(resource->info.texture.handles[0]);
                }
            }
        }
    }

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);

        if (!node->enabled) {
            continue;
        }

        for (gerium_uint32_t j = 0; j < node->inputCount; ++j) {
            auto resource = _resources.access(node->inputs[j]);

            if (resource->info.type == GERIUM_RESOURCE_TYPE_REFERENCE) {
                continue;
            }

            if (resource->info.type != GERIUM_RESOURCE_TYPE_BUFFER) {
                const auto& texture = getResource(resource->name)->info.texture;
                for (auto i = 0; i < std::size(texture.handles); ++i) {
                    resource->info.texture.handles[i] = texture.handles[i];
                }
            } else {
                resource->info.buffer.handle = getResource(resource->name)->info.buffer.handle;
            }
        }

        if (node->outputCount && !node->compute) {
            if (node->renderPass == Undefined) {
                node->renderPass = _renderer->createRenderPass(*this, node);
            }

            gerium_uint32_t maxFramebuffers = 1;
            for (auto i = 0; i < node->inputCount; ++i) {
                if (_resources.access(node->inputs[i])->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT &&
                    storedResources.contains(_resources.access(node->inputs[i])->name)) {
                    maxFramebuffers = std::size(_resources.access(node->inputs[i])->info.texture.handles);
                    break;
                }
            }
            for (auto o = 0; o < node->outputCount; ++o) {
                if (storedResources.contains(_resources.access(node->outputs[o])->name)) {
                    maxFramebuffers = std::size(_resources.access(node->outputs[o])->info.texture.handles);
                    break;
                }
            }
            for (gerium_uint32_t i = 0; i < maxFramebuffers; ++i) {
                if (node->framebuffers[i] == Undefined) {
                    node->framebuffers[i] = _renderer->createFramebuffer(*this, node, i);
                }
            }
        }
    }

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);

        if (!node->enabled) {
            continue;
        }

        if (auto it = _renderPassCache.find(hash(node->name)); it != _renderPassCache.end()) {
            node->pass = it->second;
        } else {
            _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name = node->name](auto& stream) {
                stream << "Render pass '" << name << "' not found";
            });
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err GERIUM_RESULT_ERROR_NOT_FOUND;
        }
    }

    _hasChanges = false;
}

void FrameGraph::resize(gerium_uint16_t oldWidth,
                        gerium_uint16_t newWidth,
                        gerium_uint16_t oldHeight,
                        gerium_uint16_t newHeight) {
    auto scaleX = gerium_float64_t(newWidth) / oldWidth;
    auto scaleY = gerium_float64_t(newHeight) / oldHeight;

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);

        for (auto& framebuffer : node->framebuffers) {
            if (framebuffer != Undefined) {
                _renderer->destroyFramebuffer(framebuffer);
                framebuffer = Undefined;

                _hasChanges = true;
            }
        }

        if (node->renderPass != Undefined) {
            _renderer->destroyRenderPass(node->renderPass);
            node->renderPass = Undefined;

            _hasChanges = true;
        }

        for (gerium_uint32_t j = 0; j < node->outputCount; ++j) {
            auto resource = _resources.access(node->outputs[j]);

            if (resource->external || resource->info.type == GERIUM_RESOURCE_TYPE_BUFFER) {
                continue;
            }

            if (resource->info.texture.autoScale != 0) {
                const auto scale = gerium_float64_t(resource->info.texture.autoScale);
                const auto newResourceWidth =
                    gerium_uint16_t(std::floor(resource->info.texture.width * scaleX + 0.5) * scale);
                const auto newResourceHeight =
                    gerium_uint16_t(std::floor(resource->info.texture.height * scaleY + 0.5) * scale);

                if (newResourceWidth != resource->info.texture.width ||
                    newResourceHeight != resource->info.texture.height) {
                    resource->info.texture.width  = newResourceWidth;
                    resource->info.texture.height = newResourceHeight;

                    for (auto& handle : resource->info.texture.handles) {
                        if (handle != Undefined) {
                            _renderer->destroyTexture(handle);
                            handle = Undefined;
                        }
                    }

                    _hasChanges = true;
                }
            }
        }

        if (auto pass = _renderPasses.access(node->pass); pass->pass.resize) {
            if (!pass->pass.resize(this, _renderer, pass->data)) {
                error(GERIUM_RESULT_ERROR_FROM_CALLBACK);
            }
            _hasChanges = true;
        }
    }
}

const FrameGraphResource* FrameGraph::getResource(FrameGraphResourceHandle handle) const noexcept {
    return _resources.access(handle);
}

const FrameGraphResource* FrameGraph::getResource(gerium_utf8_t name) const noexcept {
    if (auto it = _resourceCache.find(hash(name)); it != _resourceCache.end()) {
        return getResource(it->second);
    }
    return nullptr;
}

void FrameGraph::fillExternalResource(FrameGraphResourceHandle handle) noexcept {
    if (auto resource = _resources.access(handle); resource->external) {
        if (resource->saveForNextFrame) {
            _logger->print(GERIUM_LOGGER_LEVEL_WARNING, [name = resource->name](auto& stream) {
                stream << "External resource '" << name << "' cannot be set to previous_frame flag";
            });
        }

        if (auto it = _externalCache.find(hash(resource->name)); it != _externalCache.end()) {
            auto externaHandle   = it->second.handle;
            auto externalTexture = it->second.texture;
            auto resourceTexture = resource->info.type != GERIUM_RESOURCE_TYPE_BUFFER;

            if (externalTexture == resourceTexture) {
                if (resourceTexture) {
                    gerium_texture_info_t info;
                    _renderer->getTextureInfo(externaHandle, info);

                    resource->info.texture.format     = info.format;
                    resource->info.texture.width      = info.width;
                    resource->info.texture.height     = info.height;
                    resource->info.texture.depth      = info.depth;
                    resource->info.texture.handles[0] = externaHandle;
                } else {
                    resource->info.buffer.handle = externaHandle;
                }
            } else {
                _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name = resource->name](auto& stream) {
                    stream << "Cannot bind external resource because it has a different resource type '" << name << "'";
                });
            }
        } else {
            _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name = resource->name](auto& stream) {
                stream << "External resource not found '" << name << "'";
            });
        }
    }
}

gerium_uint32_t FrameGraph::nodeCount() const noexcept {
    return _nodeGraphCount;
}

const FrameGraphNode* FrameGraph::getNode(gerium_uint32_t index) const noexcept {
    return _nodes.access(_nodeGraph[index]);
}

const FrameGraphNode* FrameGraph::getNode(FrameGraphNodeHandle handle) const noexcept {
    return _nodes.access(handle);
}

const FrameGraphNode* FrameGraph::getNode(gerium_utf8_t name) const noexcept {
    if (auto it = _nodeCache.find(hash(name)); it != _nodeCache.end()) {
        return _nodes.access(it->second);
    }
    return nullptr;
}

const FrameGraphRenderPass* FrameGraph::getPass(FrameGraphRenderPassHandle handle) const noexcept {
    return _renderPasses.access(handle);
}

FrameGraphResourceHandle FrameGraph::createNodeOutput(const gerium_resource_output_t& output,
                                                      FrameGraphNodeHandle producer) {
    auto [handle, resource] = _resources.obtain_and_access();

    resource->name      = intern(output.name);
    resource->external  = output.external;
    resource->producer  = Undefined;
    resource->output    = Undefined;
    resource->info.type = output.type;

    if (output.type != GERIUM_RESOURCE_TYPE_BUFFER) {
        const auto renderPassOp =
            output.render_pass_op == GERIUM_RENDER_PASS_OP_DONT_CARE ? RenderPassOp::DontCare : RenderPassOp::Clear;

        resource->info.texture.format            = output.format;
        resource->info.texture.width             = output.width;
        resource->info.texture.height            = output.height;
        resource->info.texture.depth             = 1;
        resource->info.texture.autoScale         = output.auto_scale;
        resource->info.texture.operation         = renderPassOp;
        resource->info.texture.colorWriteMask    = output.color_write_mask;
        resource->info.texture.colorBlend        = output.color_blend_attachment;
        resource->info.texture.clearColor        = output.clear_color_attachment;
        resource->info.texture.clearDepthStencil = output.clear_depth_stencil_attachment;
        for (auto& handle : resource->info.texture.handles) {
            handle = Undefined;
        }
        calcFramebufferSize(resource->info);
    } else {
        resource->info.buffer.size      = output.size;
        resource->info.buffer.usage     = output.usage;
        resource->info.buffer.fillValue = output.fill_value;
        resource->info.buffer.handle    = Undefined;
    }

    if (output.type != GERIUM_RESOURCE_TYPE_REFERENCE) {
        resource->producer = producer;
        resource->output   = handle;

        const auto key = hash(resource->name);
        _resourceCache.insert({ key, handle });
    }

    return handle;
}

FrameGraphResourceHandle FrameGraph::createNodeInput(const gerium_resource_input_t& input) {
    auto [handle, resource] = _resources.obtain_and_access();

    resource->name             = intern(input.name);
    resource->saveForNextFrame = input.previous_frame;
    resource->producer         = Undefined;
    resource->output           = Undefined;
    resource->info.type        = input.type;

    return handle;
}

void FrameGraph::computeEdges(FrameGraphNode* node) {
    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto inputResource  = _resources.access(node->inputs[i]);
        auto outputResource = getResource(inputResource->name);

        if (outputResource == nullptr && !inputResource->external) {
            _logger->print(GERIUM_LOGGER_LEVEL_ERROR, [name = node->name](auto& stream) {
                stream << "Failed to calculate output resources for adjacent node '" << name << "'";
            });
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        const auto type = inputResource->info.type;

        inputResource->external  = outputResource->external;
        inputResource->producer  = outputResource->producer;
        inputResource->output    = outputResource->output;
        inputResource->info      = outputResource->info;
        inputResource->info.type = type;

        if (inputResource->saveForNextFrame) {
            continue;
        }

        auto perentNode                            = _nodes.access(inputResource->producer);
        perentNode->edges[perentNode->edgeCount++] = _nodes.handle(node);
    }
}

void FrameGraph::calcFramebufferSize(FrameGraphResourceInfo& info) const noexcept {
    gerium_uint16_t width, height;
    _renderer->getSwapchainSize(width, height);

    if (info.texture.width == 0) {
        info.texture.width = width;
    }

    if (info.texture.height == 0) {
        info.texture.height = height;
    }

    if (info.texture.autoScale != 0.0f) {
        info.texture.width  = gerium_uint16_t(info.texture.width * info.texture.autoScale);
        info.texture.height = gerium_uint16_t(info.texture.height * info.texture.autoScale);
    }
}

} // namespace gerium

using namespace gerium;

gerium_result_t gerium_frame_graph_create(gerium_renderer_t renderer, gerium_frame_graph_t* frame_graph) {
    assert(renderer);
    GERIUM_ASSERT_ARG(frame_graph);
    return Object::create<FrameGraph>(*frame_graph, alias_cast<Renderer*>(renderer));
}

gerium_frame_graph_t gerium_frame_graph_reference(gerium_frame_graph_t frame_graph) {
    assert(frame_graph);
    frame_graph->reference();
    return frame_graph;
}

void gerium_frame_graph_destroy(gerium_frame_graph_t frame_graph) {
    if (frame_graph) {
        frame_graph->destroy();
    }
}

gerium_result_t gerium_frame_graph_add_pass(gerium_frame_graph_t frame_graph,
                                            gerium_utf8_t name,
                                            const gerium_render_pass_t* render_pass,
                                            gerium_data_t data) {
    assert(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_ASSERT_ARG(render_pass);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<FrameGraph*>(frame_graph)->addPass(name, render_pass, data);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_frame_graph_remove_pass(gerium_frame_graph_t frame_graph, gerium_utf8_t name) {
    assert(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<FrameGraph*>(frame_graph)->removePass(name);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_frame_graph_add_node(gerium_frame_graph_t frame_graph,
                                            gerium_utf8_t name,
                                            gerium_bool_t compute,
                                            gerium_uint32_t input_count,
                                            const gerium_resource_input_t* inputs,
                                            gerium_uint32_t output_count,
                                            const gerium_resource_output_t* outputs) {
    assert(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_ASSERT_ARG(input_count == 0 || (input_count > 0 && inputs));
    GERIUM_ASSERT_ARG(output_count == 0 || (output_count > 0 && outputs));
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<FrameGraph*>(frame_graph)->addNode(name, compute, input_count, inputs, output_count, outputs);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_frame_graph_enable_node(gerium_frame_graph_t frame_graph,
                                               gerium_utf8_t name,
                                               gerium_bool_t enable) {
    assert(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<FrameGraph*>(frame_graph)->enableNode(name, enable);
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_frame_graph_add_buffer(gerium_frame_graph_t frame_graph,
                                              gerium_utf8_t name,
                                              gerium_buffer_h handle) {
    assert(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<FrameGraph*>(frame_graph)->addBuffer(name, { handle.index });
    GERIUM_END_SAFE_BLOCK
}

gerium_result_t gerium_frame_graph_add_texture(gerium_frame_graph_t frame_graph,
                                               gerium_utf8_t name,
                                               gerium_texture_h handle) {
    assert(frame_graph);
    GERIUM_ASSERT_ARG(name);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<FrameGraph*>(frame_graph)->addTexture(name, { handle.index });
    GERIUM_END_SAFE_BLOCK
}

void gerium_frame_graph_clear(gerium_frame_graph_t frame_graph) {
    assert(frame_graph);
    alias_cast<FrameGraph*>(frame_graph)->clear();
}

gerium_result_t gerium_frame_graph_compile(gerium_frame_graph_t frame_graph) {
    assert(frame_graph);
    GERIUM_BEGIN_SAFE_BLOCK
        alias_cast<FrameGraph*>(frame_graph)->compile();
    GERIUM_END_SAFE_BLOCK
}
