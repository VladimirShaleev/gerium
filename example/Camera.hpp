#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "EntityComponentSystem.hpp"
#include "Primitives.hpp"
#include "ResourceManager.hpp"

struct SceneData {
    glm::mat4 viewProjection;
    glm::vec4 eye;
};

enum class Intersection {
    None,
    Partial,
    Full
};

class Camera final : public Component {
public:
    enum Movement {
        Forward,
        Right,
        Up
    };

    Camera() = default;
    Camera(gerium_application_t application, ResourceManager& resourceManager) noexcept;

    Camera(Camera&& other)            = default;
    Camera& operator=(Camera&& other) = default;

    Camera(const Camera& other);
    Camera& operator=(const Camera& other);

    void setSpeed(gerium_float32_t movementSpeed = 0.001f, gerium_float32_t rotationSpeed = 0.001f);

    void setPosition(const glm::vec3& position);
    void setRotation(gerium_float32_t yaw, gerium_float32_t pitch);
    void setPerpective(gerium_float32_t nearPlane, gerium_float32_t farPlane, gerium_float32_t fov);
    void rotate(gerium_float32_t deltaPitch, gerium_float32_t deltaYaw, gerium_float32_t delta);
    void move(Movement direction, gerium_float32_t value, gerium_float32_t delta);
    void zoom(gerium_float32_t value, gerium_float32_t delta);

    void update(Entity& entity, gerium_data_t data) override;

    Intersection test(const glm::vec3& point) const noexcept;
    Intersection test(const BoundingBox& bbox) const noexcept;

    bool isActive() const noexcept;
    void activate() noexcept;

    const glm::mat4& view() const noexcept;
    const glm::mat4& projection() const noexcept;
    const glm::mat4& viewProjection() const noexcept;

    const glm::vec3& position() const noexcept;
    const glm::vec3& front() const noexcept;
    const glm::vec3& up() const noexcept;
    const glm::vec3& right() const noexcept;

    gerium_float32_t yaw() const noexcept;
    gerium_float32_t pitch() const noexcept;

    gerium_float32_t nearPlane() const noexcept;
    gerium_float32_t farPlane() const noexcept;
    gerium_float32_t fov() const noexcept;

    const DescriptorSet& getDecriptorSet() const noexcept;

private:
    enum FrustumPlane {
        NearFace,
        FarFace,
        RightFace,
        LeftFace,
        TopFace,
        BottomFace
    };

    void copy(const Camera& other) noexcept;

    gerium_application_t _application{};
    gerium_renderer_t _renderer{};
    ResourceManager* _resourceManager{};

    glm::vec3 _position{};
    glm::vec3 _front{};
    glm::vec3 _up{};
    glm::vec3 _right{};

    gerium_float32_t _yaw{};
    gerium_float32_t _pitch{};

    gerium_float32_t _movementSpeed;
    gerium_float32_t _rotationSpeed;

    gerium_float32_t _nearPlane;
    gerium_float32_t _farPlane;
    gerium_float32_t _fov;

    glm::mat4 _view{};
    glm::mat4 _projection{};
    glm::mat4 _viewProjection{};

    Plane _frustum[6]{};

    Buffer _data{};
    DescriptorSet _descriptorSet{};

    static Camera* _active;
};

#endif
