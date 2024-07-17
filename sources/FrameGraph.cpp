#include "FrameGraph.hpp"
#include "StringPool.hpp"

namespace gerium {

FrameGraph::FrameGraph(Renderer* renderer) : _renderer(renderer), _nodeGraphCount(0) {
}

gerium_result_t FrameGraph::addPass(gerium_utf8_t name, const gerium_render_pass_t* renderPass, gerium_data_t* data) {
    const auto key = hash(name);

    if (_renderPassCache.contains(key)) {
        return GERIUM_RESULT_ERROR_UNKNOWN; // TODO: add err GERIUM_RESULT_ERROR_EXISTS;
    }

    auto [handle, pass] = _renderPasses.obtain_and_access();

    pass->pass = *renderPass;
    pass->name = intern(name);
    pass->data = data;

    _renderPassCache.insert({ key, handle });

    return GERIUM_RESULT_SUCCESS;
}

gerium_result_t FrameGraph::removePass(gerium_utf8_t name) {
    const auto key = hash(name);

    if (auto it = _renderPassCache.find(key); it != _renderPassCache.end()) {
        _renderPassCache.erase(it);
        return GERIUM_RESULT_SUCCESS;
    }

    return GERIUM_RESULT_ERROR_UNKNOWN; // TODO: add err GERIUM_RESULT_ERROR_NOT_FOUND;
}

gerium_result_t FrameGraph::addNode(gerium_utf8_t name,
                                    gerium_uint32_t inputCount,
                                    const gerium_resource_input_t* inputs,
                                    gerium_uint32_t outputCount,
                                    const gerium_resource_output_t* outputs) {
    const auto key = hash(name);

    if (_nodeCache.contains(key)) {
        return GERIUM_RESULT_ERROR_UNKNOWN; // TODO: add err GERIUM_RESULT_ERROR_EXISTS;
    }

    auto [handle, node] = _nodes.obtain_and_access();

    node->renderPass  = Undefined;
    node->framebuffer = Undefined;
    node->pass        = Undefined;
    node->name        = intern(name);
    node->enabled     = 1;

    for (gerium_uint32_t i = 0; i < inputCount; ++i) {
        node->inputs[node->inputCount++] = createNodeInput(inputs[i]);
    }

    for (gerium_uint32_t i = 0; i < outputCount; ++i) {
        node->outputs[node->outputCount++] = createNodeOutput(outputs[i], handle);
    }

    _nodeGraph[_nodeGraphCount++] = handle;
    _nodeCache.insert({ key, handle });

    return GERIUM_RESULT_SUCCESS;
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

gerium_uint32_t FrameGraph::nodeCount() const noexcept {
    return _nodeGraphCount;
}

const FrameGraphNode* FrameGraph::getNode(gerium_uint32_t index) const noexcept {
    return _nodes.access(_nodeGraph[index]);
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

    resource->name                   = intern(output.name);
    resource->external               = output.external;
    resource->producer               = Undefined;
    resource->output                 = Undefined;
    resource->info.type              = output.type;
    resource->info.texture.format    = output.format;
    resource->info.texture.width     = 800; // output.width;
    resource->info.texture.height    = 600; // output.height;
    resource->info.texture.depth     = 1;
    resource->info.texture.operation = output.operation;
    resource->info.texture.handle    = Undefined;

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

    resource->name      = intern(input.name);
    resource->producer  = Undefined;
    resource->output    = Undefined;
    resource->info.type = input.type;

    return handle;
}

void FrameGraph::compileGraph() {
    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);
        if (node->enabled) {
            computeEdges(node);
        }
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

    std::copy_n(_sortedNodes.crbegin() + (kMaxNodes - _nodeGraphCount), sortedNodeCount, _nodeGraph.begin());

    for (auto& item : _allocations) {
        item = Undefined;
    }

    gerium_uint32_t _freeListCount = 0;

    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);

        if (!node->enabled) {
            continue;
        }

        for (gerium_uint32_t j = 0; j < node->inputCount; ++j) {
            auto inputResource  = _resources.access(node->inputs[j]);
            auto outputResource = _resources.access(inputResource->output);
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
                    creation.setName(resource->name)
                        .setFormat(info.format, GERIUM_TEXTURE_TYPE_2D)
                        .setSize(info.width, info.height, info.depth)
                        .setFlags(1, true, false);

                    if (_freeListCount) {
                        auto alias = _freeList[--_freeListCount];
                        creation.setAlias(alias);
                    }

                    error(_renderer->createTexture(creation, resource->info.texture.handle));
                }
            }
        }

        for (gerium_uint32_t j = 0; j < node->inputCount; ++j) {
            auto inputResource = _resources.access(node->inputs[j]);

            auto resourceIndex = inputResource->output.index;
            auto resource      = _resources.access(inputResource->output);

            --resource->refCount;

            if (!resource->external && resource->refCount == 0) {
                if (resource->info.type == GERIUM_RESOURCE_TYPE_ATTACHMENT ||
                    resource->info.type == GERIUM_RESOURCE_TYPE_TEXTURE) {
                    _freeList[_freeListCount++] = resource->info.texture.handle;
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

            if (resource->info.type == GERIUM_RESOURCE_TYPE_BUFFER ||
                resource->info.type == GERIUM_RESOURCE_TYPE_REFERENCE) {
                continue;
            }

            resource->info.texture.handle = getResource(resource->name)->info.texture.handle;
        }

        if (node->outputCount) {
            if (node->renderPass == Undefined) {
                error(_renderer->createRenderPass(*this, node, node->renderPass));
            }

            if (node->framebuffer == Undefined) {
                error(_renderer->createFramebuffer(*this, node, node->framebuffer));
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
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err GERIUM_RESULT_ERROR_NOT_FOUND;
        }
    }
}

void FrameGraph::computeEdges(FrameGraphNode* node) {
    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto inputResource  = _resources.access(node->inputs[i]);
        auto outputResource = getResource(inputResource->name);

        if (outputResource == nullptr && !inputResource->external) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        const auto type = inputResource->info.type;

        inputResource->external  = outputResource->external;
        inputResource->producer  = outputResource->producer;
        inputResource->output    = outputResource->output;
        inputResource->info      = outputResource->info;
        inputResource->info.type = type;

        auto perentNode                            = _nodes.access(inputResource->producer);
        perentNode->edges[perentNode->edgeCount++] = _nodes.handle(node);
    }
}

void FrameGraph::clearGraph() noexcept {
    _nodes.releaseAll();
    _resources.releaseAll();
    _nodeCache.clear();
    _resourceCache.clear();
    _nodeGraphCount = 0;
}

} // namespace gerium

using namespace gerium;

gerium_result_t gerium_frame_graph_create(gerium_renderer_t renderer, gerium_frame_graph_t* frame_graph) {
    assert(renderer);
    assert(frame_graph);
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
                                            gerium_data_t* data) {
    assert(frame_graph);
    assert(name);
    assert(render_pass);
    return alias_cast<FrameGraph*>(frame_graph)->addPass(name, render_pass, data);
}

gerium_result_t gerium_frame_graph_remove_pass(gerium_frame_graph_t frame_graph, gerium_utf8_t name) {
    assert(frame_graph);
    assert(name);
    return alias_cast<FrameGraph*>(frame_graph)->removePass(name);
}

gerium_result_t gerium_frame_graph_add_node(gerium_frame_graph_t frame_graph,
                                            gerium_utf8_t name,
                                            gerium_uint32_t input_count,
                                            const gerium_resource_input_t* inputs,
                                            gerium_uint32_t output_count,
                                            const gerium_resource_output_t* outputs) {
    assert(frame_graph);
    assert(name);
    assert(input_count == 0 || (input_count > 0 && inputs));
    assert(output_count == 0 || (output_count > 0 && outputs));
    return alias_cast<FrameGraph*>(frame_graph)->addNode(name, input_count, inputs, output_count, outputs);
}

gerium_result_t gerium_frame_graph_compile(gerium_frame_graph_t frame_graph) {
    assert(frame_graph);
    return alias_cast<FrameGraph*>(frame_graph)->compile();
}
