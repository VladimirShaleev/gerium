#include "FrameGraph.hpp"
#include "StringPool.hpp"

namespace gerium {

FrameGraph::FrameGraph() : _nodeGraphCount(0) {
}

gerium_result_t FrameGraph::addPass(gerium_utf8_t name, const gerium_render_pass_t* renderPass, gerium_data_t* data) {
    const auto key = hash(name);

    if (_renderPassCache.contains(key)) {
        return GERIUM_RESULT_ERROR_UNKNOWN; // TODO: add err GERIUM_RESULT_ERROR_EXISTS;
    }

    auto [handle, pass] = _rendererPasses.obtain_and_access();

    pass->pass = renderPass;
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
    // node->pass        = nullptr;
    node->name = intern(name);
    // node->edgeCount   = 0;
    node->enabled = 1;
    // node->edges       = ;

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

FrameGraphResourceHandle FrameGraph::createNodeOutput(const gerium_resource_output_t& output,
                                                      FrameGraphNodeHandle producer) {
    auto [handle, resource] = _resources.obtain_and_access();

    resource->type      = output.type;
    resource->name      = intern(output.name);
    resource->external  = output.external;
    resource->format    = output.format;
    resource->width     = output.width;
    resource->height    = output.height;
    resource->operation = output.operation;
    resource->producer  = Undefined;
    resource->output    = Undefined;

    if (resource->type != GERIUM_RESOURCE_TYPE_REFERENCE) {
        resource->producer = producer;
        resource->output   = handle;

        const auto key = hash(resource->name);
        _resourceCache.insert({ key, handle });
    }

    return handle;
}

FrameGraphResourceHandle FrameGraph::createNodeInput(const gerium_resource_input_t& input) {
    auto [handle, resource] = _resources.obtain_and_access();

    resource->type     = input.type;
    resource->name     = intern(input.name);
    resource->producer = Undefined;
    resource->output   = Undefined;

    return handle;
}

void FrameGraph::compileGraph() {
    for (gerium_uint32_t i = 0; i < _nodeGraphCount; ++i) {
        auto node = _nodes.access(_nodeGraph[i]);
        if (node->enabled) {
            computeEdges(node);
        }
    }
}

void FrameGraph::computeEdges(FrameGraphNode* node) {
    for (gerium_uint32_t i = 0; i < node->inputCount; ++i) {
        auto inputResource  = _resources.access(node->inputs[i]);
        auto outputResource = findResource(inputResource->name);

        if (outputResource == nullptr && !inputResource->external) {
            error(GERIUM_RESULT_ERROR_UNKNOWN); // TODO: add err
        }

        inputResource->external  = outputResource->external;
        inputResource->format    = outputResource->format;
        inputResource->width     = outputResource->width;
        inputResource->height    = outputResource->height;
        inputResource->operation = outputResource->operation;
        inputResource->producer  = outputResource->producer;
        inputResource->output    = outputResource->output;

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

FrameGraphResource* FrameGraph::findResource(gerium_utf8_t name) noexcept {
    if (auto it = _resourceCache.find(hash(name)); it != _resourceCache.end()) {
        return _resources.access(it->second);
    }
    return nullptr;
}

} // namespace gerium

using namespace gerium;

gerium_result_t gerium_frame_graph_create(gerium_renderer_t renderer, gerium_frame_graph_t* frame_graph) {
    assert(renderer);
    assert(frame_graph);
    return Object::create<FrameGraph>(*frame_graph);
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
