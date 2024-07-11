#include "VkProfiler.hpp"
#include "../StringPool.hpp"
#include "Device.hpp"

namespace gerium::vulkan {

VkProfiler::VkProfiler(Device& device, uint16_t queriesPerFrame, uint16_t maxFrames) :
    _device(&device),
    _queriesPerFrame(queriesPerFrame),
    _currentQuery(0),
    _parentQuery(0),
    _depth(0),
    _maxFrames(maxFrames) {
    _timestamps.resize(_queriesPerFrame * _maxFrames);
    _timestampsData.resize(_queriesPerFrame * _maxFrames * 2);
}

uint32_t VkProfiler::pushTimestamp(gerium_utf8_t name) {
    const uint32_t queryIndex = (_device->currentFrame() * _queriesPerFrame) + _currentQuery;

    auto& timestamp  = _timestamps[queryIndex];
    timestamp.start  = queryIndex * 2;
    timestamp.end    = timestamp.start + 1;
    timestamp.parent = (uint16_t) _parentQuery;
    timestamp.depth  = (uint16_t) _depth++;
    timestamp.name   = intern(name);

    _parentQuery = _currentQuery;
    ++_currentQuery;

    return timestamp.start;
}

uint32_t VkProfiler::popTimestamp() {
    uint32_t queryIndex = (_device->currentFrame() * _queriesPerFrame) + _parentQuery;

    auto& timestamp = _timestamps[queryIndex];

    _parentQuery = timestamp.parent;
    --_depth;

    return timestamp.end;
}

void VkProfiler::resetTimestamps() {
    _currentQuery = 0;
    _parentQuery  = 0;
    _depth        = 0;
}

bool VkProfiler::hasTimestamps() const noexcept {
    return _currentQuery > 0 && _depth == 0;
}

// const VkProfiler::Timestamp& VkProfiler::getTimestamp(uint32_t index) const noexcept {
//     return _timestamps[index];
// }
//
// const uint64_t* VkProfiler::getTimestampData(uint32_t offset) const noexcept {
//     return &_timestampsData[offset];
// }

uint16_t VkProfiler::queriesPerFrame() const noexcept {
    return _queriesPerFrame;
}

// uint16_t VkProfiler::maxFrames() const noexcept {
//     return _maxFrames;
// }

// uint32_t VkProfiler::queryCount() const noexcept {
//     return _currentQuery;
// }
//
// uint32_t VkProfiler::queryIndex(uint32_t query) const noexcept {
//     auto index = _device->currentFrame() * _queriesPerFrame + query;
// }

void VkProfiler::fetchDataFromGpu() {
    if (hasTimestamps()) {
        const auto queryOffset = _device->currentFrame() * _queriesPerFrame * 2;
        const auto queryCount  = _currentQuery * 2;

        check(_device->vkTable().vkGetQueryPoolResults(_device->vkDevice(),
                                                       _device->vkQueryPool(),
                                                       queryOffset,
                                                       queryCount,
                                                       sizeof(uint64_t) * queryCount * 2,
                                                       &_timestampsData[queryOffset],
                                                       sizeof(uint64_t),
                                                       VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT));

        for (uint32_t q = 0; q < _currentQuery; ++q) {
            auto index = _device->currentFrame() * _queriesPerFrame + q;

            auto& timestamp = _timestamps[index];

            double start   = (double) _timestampsData[index * 2];
            double end     = (double) _timestampsData[index * 2 + 1];
            double range   = end - start;
            double elapsed = range * _device->gpuFrequency();

            timestamp.frame   = _device->absoluteFrame();
            timestamp.elapsed = elapsed;
        }
    }
}

} // namespace gerium::vulkan
