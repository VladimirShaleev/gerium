#include "Profiler.hpp"
#include "Renderer.hpp"

namespace gerium {

void Profiler::getGpuTimestamps(gerium_uint32_t& gpuTimestampsCount,
                                gerium_gpu_timestamp_t* gpuTimestamps) const noexcept {
    onGetGpuTimestamps(gpuTimestampsCount, gpuTimestamps);
}

gerium_uint32_t Profiler::getGpuTotalMemoryUsed() const noexcept {
    return onGetGpuTotalMemoryUsed();
}

} // namespace gerium

using namespace gerium;

gerium_result_t gerium_profiler_create(gerium_renderer_t renderer, gerium_profiler_t* profiler) {
    assert(renderer);
    assert(profiler);
    auto result = alias_cast<Renderer*>(renderer)->getProfiler();
    *profiler   = gerium_profiler_reference(result);
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

void gerium_profiler_get_gpu_timestamps(gerium_profiler_t profiler,
                                        gerium_uint32_t* gpu_timestamps_count,
                                        gerium_gpu_timestamp_t* gpu_timestamps) {
    assert(profiler);
    assert(gpu_timestamps_count);
    return alias_cast<Profiler*>(profiler)->getGpuTimestamps(*gpu_timestamps_count, gpu_timestamps);
}

gerium_uint32_t gerium_profiler_get_gpu_total_memory_used(gerium_profiler_t profiler) {
    assert(profiler);
    return alias_cast<Profiler*>(profiler)->getGpuTotalMemoryUsed();
}
