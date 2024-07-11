#include "Profiler.hpp"
#include "Renderer.hpp"

namespace gerium {} // namespace gerium

using namespace gerium;

gerium_result_t gerium_profiler_create(gerium_renderer_t renderer, gerium_profiler_t* profiler) {
    assert(renderer);
    assert(profiler);
    auto result = alias_cast<Renderer*>(renderer)->getProfiler();
    *profiler = gerium_profiler_reference(result);
    return GERIUM_RESULT_SUCCESS;
}

gerium_profiler_t gerium_profiler_reference(gerium_profiler_t profiler) {
    assert(profiler);
    profiler->reference();
    return profiler;
}

void gerium_profiler_destroy(gerium_profiler_t profiler) {
    if (profiler) {
        profiler->destroy();
    }
}
