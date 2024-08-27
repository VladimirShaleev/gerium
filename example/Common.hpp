#ifndef COMMON_HPP
#define COMMON_HPP

#define _USE_MATH_DEFINES

#include <gerium/gerium.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

#include "Finally.hpp"

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
#define GLM_FORCE_SWIZZLE 
// #define GLM_FORCE_MESSAGES
// https://github.com/g-truc/glm/issues/1269
#include <glm/detail/setup.hpp>
#undef GLM_DEPRECATED
#define GLM_DEPRECATED [[deprecated]]
#include <glm/ext.hpp>

#include <stb_image.h>

#include <wyhash.h>

#include <magic_enum.hpp>

static constexpr gerium_uint16_t UndefinedHandle = std::numeric_limits<gerium_uint16_t>::max();

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

enum class Axis {
    X,
    Y,
    Z,
    NONE
};

struct Plane {
    glm::vec3 normal;
    gerium_float32_t distance;

    void normalize() noexcept {
        auto invLength = 1.0f / glm::length(normal);
        normal *= invLength;
        distance *= invLength;
    }

    gerium_float32_t getDistanceToPlane(const glm::vec3& point) const noexcept {
        return glm::dot(normal, point) + distance;
    }
};

class BoundingBox {
public:
    BoundingBox() = default;

    BoundingBox(const glm::vec3& min, const glm::vec3& max) noexcept : _min(min), _max(max) {
    }

    const glm::vec3& min() const noexcept {
        return _min;
    }

    const glm::vec3& max() const noexcept {
        return _max;
    }

    BoundingBox combine(const BoundingBox& bbox) const noexcept {
        return BoundingBox(glm::min(min(), bbox.min()), glm::max(max(), bbox.max()));
    }

    float sa() const noexcept {
        const auto sl = max() - min();
        return (sl.x * sl.y) + (sl.x * sl.z) + (sl.y * sl.z);
    }

    float getWidth(Axis axis) const noexcept {
        switch (axis) {
            case Axis::X:
                return max().x - min().x;
            case Axis::Y:
                return max().y - min().y;
            case Axis::Z:
                return max().z - min().z;
            default:
                return 0.0f;
        }
    }

    float getCenter(Axis axis) const noexcept {
        switch (axis) {
            case Axis::X:
                return (max().x + min().x) * 0.5f;
            case Axis::Y:
                return (max().y + min().y) * 0.5f;
            case Axis::Z:
                return (max().z + min().z) * 0.5f;
            default:
                return 0.0f;
        }
    }

    float getAxisStart(Axis axis) const noexcept {
        switch (axis) {
            case Axis::X:
                return min().x;
            case Axis::Y:
                return min().y;
            case Axis::Z:
                return min().z;
            default:
                return 0.0f;
        }
    }

    glm::vec3 getCentroid() const noexcept {
        return (max() + min()) * 0.5f;
    }

private:
    glm::vec3 _min{ std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::max() };
    glm::vec3 _max{ std::numeric_limits<float>::min(),
                    std::numeric_limits<float>::min(),
                    std::numeric_limits<float>::min() };
};

#endif
