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

inline std::string& ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](auto c) {
        return !std::isspace(c);
    }));
    return s;
}

inline std::string& rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(),
                         s.rend(),
                         [](auto ch) {
        return !std::isspace(ch);
    }).base(),
            s.end());
    return s;
}

inline std::string& trim(std::string& s) {
    rtrim(s);
    ltrim(s);
    return s;
}

#endif // UTILS_FUNCTIONS_HPP
