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
