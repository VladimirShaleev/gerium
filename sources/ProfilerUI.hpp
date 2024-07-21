#ifndef GERIUM_PROFILER_UI_HPP
#define GERIUM_PROFILER_UI_HPP

#include "Profiler.hpp"

namespace gerium {

class ProfilerUI final {
public:
    void draw(Profiler* profiler, bool* show, uint32_t maxFrames);

private:
    std::vector<gerium_gpu_timestamp_t> timestamps;
    std::vector<uint32_t> colors;
    std::vector<uint32_t> perFrameActive;

    uint32_t currentFrame{};

    float maxTime{};
    float minTime{};
    float averageTime{};

    float maxDuration{};
    bool prevPaused{};
    bool paused{};

    //uint32_t initialFramesPaused = 3;

    std::map<uint64_t, uint32_t> nameToColor;

    uint32_t totalMemoryUsed;
};

} // namespace gerium

#endif
