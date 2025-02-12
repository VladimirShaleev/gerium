#ifndef UTILS_PRIMITIVES_HPP
#define UTILS_PRIMITIVES_HPP

#include "../Common.hpp"

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

#endif // UTILS_PRIMITIVES_HPP
