#include "FrameGraph.hpp"

namespace gerium {} // namespace gerium

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
    return GERIUM_RESULT_ERROR_NOT_IMPLEMENTED;
}

gerium_result_t gerium_frame_graph_add_node(gerium_frame_graph_t frame_graph,
                                            gerium_utf8_t name,
                                            gerium_uint32_t input_count,
                                            const gerium_frame_graph_resource_t* inputs,
                                            gerium_uint32_t output_count,
                                            const gerium_frame_graph_resource_t* outputs) {
    assert(frame_graph);
    assert(name);
    assert(input_count == 0 || (input_count > 0 && inputs));
    assert(output_count == 0 || (output_count > 0 && outputs));
    return GERIUM_RESULT_ERROR_NOT_IMPLEMENTED;
}
