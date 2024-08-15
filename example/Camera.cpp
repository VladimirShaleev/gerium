#include "Camera.hpp"

Camera::Camera(gerium_application_t application, gerium_renderer_t renderer) noexcept :
    _application(application),
    _renderer(renderer) {
    setSpeed();
    setPosition({ 0.0f, 0.0f, 0.0f });
    setRotation(0.0f, 0.0f);
    setPerpective(0.01f, 1000.0f, glm::radians(60.0f));

    check(gerium_renderer_create_buffer(
        _renderer, GERIUM_BUFFER_USAGE_UNIFORM_BIT, 1, "scene_data", nullptr, sizeof(SceneData), &_data));
    check(gerium_renderer_create_descriptor_set(_renderer, &_descriptorSet));
    gerium_renderer_bind_buffer(_renderer, _descriptorSet, 0, _data);

    update();
}

Camera::~Camera() {
    if (_data.unused != UndefinedHandle) {
        gerium_renderer_destroy_buffer(_renderer, _data);
        _data = { UndefinedHandle };
    }
    if (_descriptorSet.unused != UndefinedHandle) {
        gerium_renderer_destroy_descriptor_set(_renderer, _descriptorSet);
        _descriptorSet = { UndefinedHandle };
    }
}

void Camera::setSpeed(gerium_float32_t movementSpeed, gerium_float32_t rotationSpeed) {
    _movementSpeed = movementSpeed;
    _rotationSpeed = rotationSpeed;
}

void Camera::setPosition(const glm::vec3& position) {
    _position = position;
}

void Camera::setRotation(gerium_float32_t yaw, gerium_float32_t pitch) {
    _yaw   = yaw;
    _pitch = pitch;
}

void Camera::setPerpective(gerium_float32_t nearPlane, gerium_float32_t farPlane, gerium_float32_t fov) {
    _nearPlane = nearPlane;
    _farPlane  = farPlane;
    _fov       = fov;
}

void Camera::rotate(gerium_float32_t deltaPitch, gerium_float32_t deltaYaw, gerium_float32_t delta) {
    auto speed = _fov / glm::radians(30.0f);
    _pitch += deltaPitch * _rotationSpeed * speed * delta;
    _yaw += deltaYaw * _rotationSpeed * speed * delta;
}

void Camera::move(Movement direction, gerium_float32_t value, gerium_float32_t delta) {
    auto velocity = _movementSpeed * delta;
    if (direction == Forward) {
        _position += _front * value * velocity;
    } else if (direction == Right) {
        _position += _right * value * velocity;
    } else if (direction == Up) {
        _position += glm::vec3(0.0f, 1.0f, 0.0f) * value * velocity;
    }
}

void Camera::zoom(gerium_float32_t value, gerium_float32_t delta) {
    _fov += value * delta;
}

void Camera::update() {
    _fov   = std::clamp(_fov, glm::radians(30.0f), glm::radians(120.0f));
    _pitch = std::clamp(_pitch, glm::radians(-89.99f), glm::radians(89.99f));
    _yaw   = std::fmod(_yaw, M_PI * 2.0);
    if (_yaw < 0.0f) {
        _yaw += M_PI * 2.0;
    }

    _front.x = cos(_yaw) * cos(_pitch);
    _front.y = sin(_pitch);
    _front.z = sin(_yaw) * cos(_pitch);
    _front   = glm::normalize(_front);
    _right   = glm::normalize(glm::cross(_front, glm::vec3(0.0f, 1.0f, 0.0f)));
    _up      = glm::normalize(glm::cross(_right, _front));

    gerium_uint16_t width, height;
    gerium_application_get_size(_application, &width, &height);

    const auto pitch    = glm::angleAxis(_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    const auto yaw      = glm::angleAxis(_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    const auto rotation = glm::normalize(pitch * yaw);

    const auto R = glm::mat4_cast(rotation);
    const auto T = glm::translate(glm::identity<glm::mat4>(), _position);

    const auto aspect = float(width) / height;

    _projection     = glm::perspective(_fov, aspect, _nearPlane, _farPlane);
    _view           = glm::lookAt(_position, _position + _front, _up);
    _viewProjection = _projection * _view;

    auto data            = (SceneData*) gerium_renderer_map_buffer(_renderer, _data, 0, 0);
    data->viewProjection = _viewProjection;
    data->eye            = glm::vec4(_front, 1.0f);
    gerium_renderer_unmap_buffer(_renderer, _data);
}

const glm::mat4& Camera::view() const noexcept {
    return _view;
}

const glm::mat4& Camera::projection() const noexcept {
    return _projection;
}

const glm::mat4& Camera::viewProjection() const noexcept {
    return _viewProjection;
}

const glm::vec3& Camera::position() const noexcept {
    return _position;
}

const glm::vec3& Camera::front() const noexcept {
    return _front;
}

const glm::vec3& Camera::up() const noexcept {
    return _up;
}

const glm::vec3& Camera::right() const noexcept {
    return _right;
}

gerium_float32_t Camera::yaw() const noexcept {
    return _yaw;
}

gerium_float32_t Camera::pitch() const noexcept {
    return _pitch;
}

gerium_float32_t Camera::nearPlane() const noexcept {
    return _nearPlane;
}

gerium_float32_t Camera::farPlane() const noexcept {
    return _farPlane;
}

gerium_float32_t Camera::fov() const noexcept {
    return _fov;
}

gerium_descriptor_set_h Camera::getDecriptorSet() const noexcept {
    return _descriptorSet;
}
