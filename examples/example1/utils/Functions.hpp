#ifndef UTILS_FUNCTIONS_HPP
#define UTILS_FUNCTIONS_HPP

#include "../Common.hpp"

inline void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS && result != GERIUM_RESULT_SKIP_FRAME) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

template <typename T>
inline T getGroupCount(T threadCount, T localSize) noexcept {
    return (threadCount + localSize - 1) / localSize;
}

template <typename T>
inline gerium_uint32_t calcMipLevels(T width, T height) noexcept {
    static_assert(std::is_integral_v<T>, "T is not integral type");
    return static_cast<gerium_uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

#endif // UTILS_FUNCTIONS_HPP
