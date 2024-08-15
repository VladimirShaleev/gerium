#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "Common.hpp"

struct SceneData {
    glm::mat4 viewProjection;
    glm::vec4 eye;
};

class Camera final {
public:
    explicit Camera(gerium_application_t application, gerium_renderer_t renderer) noexcept;
    ~Camera();

    Camera(const Camera&) = delete;
    Camera(Camera&&)      = delete;

    Camera& operator=(const Camera&) = delete;
    Camera& operator=(Camera&&)      = delete;

    void setSpeed(gerium_float32_t rotationSpeed = 10.f,
                  gerium_float32_t movementSpeed = 10.f,
                  gerium_float32_t movementDelta = 0.1f);

    void setPosition(const glm::vec3& position);
    void setRotation(gerium_float32_t yaw, gerium_float32_t pitch);
    void setPerpective(gerium_float32_t nearPlane, gerium_float32_t farPlane, gerium_float32_t fov);
    void rotate(gerium_float32_t deltaPitch, gerium_float32_t deltaYaw, gerium_float32_t delta);
    void move(gerium_float32_t forward, gerium_float32_t up, gerium_float32_t right, gerium_float32_t delta);

    void update();

    const glm::mat4& view() const noexcept;
    const glm::mat4& projection() const noexcept;
    const glm::mat4& viewProjection() const noexcept;

    glm::vec3 up() const noexcept;
    glm::vec3 right() const noexcept;
    glm::vec3 direction() const noexcept;

    gerium_descriptor_set_h getDecriptorSet() const noexcept;

private:
    gerium_application_t _application{};
    gerium_renderer_t _renderer{};

    gerium_float32_t _rotationSpeed{};
    gerium_float32_t _movementSpeed{};
    gerium_float32_t _movementDelta{};

    glm::vec3 _position{};
    gerium_float32_t _yaw{};
    gerium_float32_t _pitch{};

    gerium_float32_t _nearPlane;
    gerium_float32_t _farPlane;
    gerium_float32_t _fov;

    glm::vec3 _up{};
    glm::vec3 _right{};
    glm::vec3 _direction{};

    glm::mat4 _view{};
    glm::mat4 _projection{};
    glm::mat4 _viewProjection{};

    gerium_buffer_h _data{ UndefinedHandle };
    gerium_descriptor_set_h _descriptorSet{ UndefinedHandle };
};

#endif
