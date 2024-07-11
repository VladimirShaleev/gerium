#ifndef GERIUM_PROFILER_HPP
#define GERIUM_PROFILER_HPP

#include "ObjectPtr.hpp"

struct _gerium_profiler : public gerium::Object {};

namespace gerium {

class Profiler : public _gerium_profiler {
public:
    void getGpuTimestamps(gerium_uint32_t& gpuTimestampsCount, gerium_gpu_timestamp_t* gpuTimestamps) const noexcept;

    gerium_uint32_t getGpuTotalMemoryUsed() const noexcept;

private:
    virtual void onGetGpuTimestamps(gerium_uint32_t& gpuTimestampsCount,
                                    gerium_gpu_timestamp_t* gpuTimestamps) const noexcept = 0;

    virtual gerium_uint32_t onGetGpuTotalMemoryUsed() const noexcept = 0;
};

} // namespace gerium

#endif
