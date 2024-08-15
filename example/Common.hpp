#ifndef COMMON_HPP
#define COMMON_HPP

#define _USE_MATH_DEFINES

#include <gerium/gerium.h>

#include <cmath>
#include <filesystem>
#include <limits>
#include <memory>
#include <nlohmann/json.hpp>
#include <queue>
#include <string_view>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_MESSAGES
// https://github.com/g-truc/glm/issues/1269
#include <glm/detail/setup.hpp>
#undef GLM_DEPRECATED
#define GLM_DEPRECATED [[deprecated]]
#include <glm/ext.hpp>

static constexpr gerium_uint16_t UndefinedHandle = std::numeric_limits<gerium_uint16_t>::max();

inline void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS && result != GERIUM_RESULT_SKIP_FRAME) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

#endif
