#ifndef COMMON_HPP
#define COMMON_HPP

#define _USE_MATH_DEFINES

#include <gerium/gerium.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include <cmath>
#include <filesystem>
#include <limits>
#include <memory>
#include <queue>
#include <string_view>

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_SWIZZLE
// #define GLM_FORCE_MESSAGES
// https://github.com/g-truc/glm/issues/1269
#include <glm/detail/setup.hpp>
#undef GLM_DEPRECATED
#define GLM_DEPRECATED [[deprecated]]
#include <glm/ext.hpp>

#include <stb_image.h>

#include <wyhash.h>

#define MAGIC_ENUM_RANGE_MAX 255
#include <magic_enum/magic_enum.hpp>

#include "Finally.hpp"
#include "shaders/common/types.h"

static constexpr gerium_uint16_t UndefinedHandle = std::numeric_limits<gerium_uint16_t>::max();

inline int typeIdSequence = 0;
template <typename T>
inline const int typeId = typeIdSequence++;

inline void check(gerium_result_t result) {
    if (result != GERIUM_RESULT_SUCCESS && result != GERIUM_RESULT_SKIP_FRAME) {
        throw std::runtime_error(gerium_result_to_string(result));
    }
}

template <typename T>
inline gerium_uint32_t calcMipLevels(T width, T height) noexcept {
    static_assert(std::is_integral_v<T>, "T is not integral type");
    return static_cast<gerium_uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
}

inline gerium_float32_t halton(gerium_sint32_t i, gerium_sint32_t b) noexcept {
    auto f  = 1.0f;
    auto r  = 0.0f;
    auto fb = gerium_float32_t(b);
    while (i > 0) {
        f = f / fb;
        r = r + f * gerium_float32_t(i % b);
        i = i / b;
    }
    return r;
}

inline glm::vec2 halton23(gerium_sint32_t index) noexcept {
    return { halton(index + 1, 2), halton(index + 1, 3) };
}

inline glm::vec2 mRobertR2(gerium_sint32_t index) noexcept {
    const auto g  = 1.32471795724474602596f;
    const auto a1 = 1.0f / g;
    const auto a2 = 1.0f / (g * g);

    const auto x = std::fmod(0.5f + a1 * index, 1.0f);
    const auto y = std::fmod(0.5f + a2 * index, 1.0f);
    return { x, y };
}

inline gerium_float32_t interleavedGradientNoise(const glm::vec2& pixel, gerium_sint32_t index) noexcept {
    const auto newPixel = pixel + gerium_float32_t(index) * 5.588238f;
    return std::fmod(52.9829189f * fmodf(0.06711056f * newPixel.x + 0.00583715f * newPixel.y, 1.0f), 1.0f);
}

inline glm::vec2 interleavedGradient(gerium_sint32_t index) noexcept {
    return { interleavedGradientNoise({ 1.f, 1.f }, index), interleavedGradientNoise({ 1.f, 2.f }, index) };
}

inline gerium_float32_t radicalInverseBase2(gerium_uint32_t bits) noexcept {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return gerium_float32_t(bits) * 2.3283064365386963e-10f;
}

inline glm::vec2 hammersley(gerium_sint32_t index, gerium_sint32_t numSamples) noexcept {
    return { gerium_float32_t(index) / numSamples, radicalInverseBase2(gerium_uint32_t(index)) };
}

#endif
