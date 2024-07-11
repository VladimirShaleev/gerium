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
    // const Timestamp& getTimestamp(uint32_t index) const noexcept;
    // const uint64_t* getTimestampData(uint32_t offset) const noexcept;

    uint16_t queriesPerFrame() const noexcept;
    // uint16_t maxFrames() const noexcept;
    // uint32_t queryCount() const noexcept;
    // uint32_t queryIndex(uint32_t query) const noexcept;

    void fetchDataFromGpu();

private:
    Device* _device;

    uint16_t _queriesPerFrame;
    uint32_t _currentQuery;
    uint32_t _parentQuery;
    uint32_t _depth;
    uint16_t _maxFrames;

    std::vector<Timestamp> _timestamps;
    std::vector<uint64_t> _timestampsData;
};

} // namespace gerium::vulkan

#endif
