#ifndef GERIUM_WINDOWS_VULKAN_VK_PROFILER_HPP
#define GERIUM_WINDOWS_VULKAN_VK_PROFILER_HPP

#include "../Profiler.hpp"

namespace gerium::vulkan {

class Device;

class VkProfiler : public Profiler {
public:
    struct Timestamp {
        uint32_t start;
        uint32_t end;
        uint16_t parent;
        uint16_t depth;
        uint32_t frame;
        double elapsed;
        gerium_utf8_t name;
    };

    VkProfiler(Device& device, uint16_t queriesPerFrame, uint16_t maxFrames);

    uint32_t pushTimestamp(gerium_utf8_t name);
    uint32_t popTimestamp();
    void resetTimestamps();
    bool hasTimestamps() const noexcept;

    uint16_t queriesPerFrame() const noexcept;

    void fetchDataFromGpu();

private:
    void onGetGpuTimestamps(gerium_uint32_t& gpuTimestampsCount,
                            gerium_gpu_timestamp_t* gpuTimestamps) const noexcept override;

    Device* _device;

    uint16_t _queriesPerFrame;
    uint32_t _currentQuery;
    uint32_t _parentQuery;
    uint32_t _depth;

    std::vector<Timestamp> _timestamps;
    std::vector<uint64_t> _timestampsData;
};

} // namespace gerium::vulkan

#endif
