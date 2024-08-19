#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Common.hpp"

struct SceneData {
    glm::mat4 viewProjection;
    glm::vec4 eye;
};

class Camera final {
public:
    enum Movement {
        Forward,
        Right,
        Up
    };

    Camera() = default;
    Camera(gerium_application_t application, gerium_renderer_t renderer) noexcept;
    ~Camera();

    Camera(const Camera& other);
    Camera(Camera&& other);

    Camera& operator=(const Camera& other);
    Camera& operator=(Camera&& other);

    void setSpeed(gerium_float32_t movementSpeed = 0.001f, gerium_float32_t rotationSpeed = 0.001f);

    void setPosition(const glm::vec3& position);
    void setRotation(gerium_float32_t yaw, gerium_float32_t pitch);
    void setPerpective(gerium_float32_t nearPlane, gerium_float32_t farPlane, gerium_float32_t fov);
    void rotate(gerium_float32_t deltaPitch, gerium_float32_t deltaYaw, gerium_float32_t delta);
    void move(Movement direction, gerium_float32_t value, gerium_float32_t delta);
    void zoom(gerium_float32_t value, gerium_float32_t delta);

    void update();

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

    gerium_descriptor_set_h getDecriptorSet() const noexcept;

private:
    void copy(const Camera& other) noexcept;

    gerium_application_t _application{};
    gerium_renderer_t _renderer{};

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

    gerium_buffer_h _data{ UndefinedHandle };
    gerium_descriptor_set_h _descriptorSet{ UndefinedHandle };
};

#endif
