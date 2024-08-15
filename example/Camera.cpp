#include "Camera.hpp"

Camera::Camera(gerium_application_t application, gerium_renderer_t renderer) noexcept :
    _application(application),
    _renderer(renderer) {
    setSpeed();
    setPosition({ 0.0f, 0.0f, 0.0f });
    setRotation(0.0f, 0.0f);
    setPerpective(0.1, 10000.0f, glm::radians(60.0f));

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

void Camera::setSpeed(gerium_float32_t rotationSpeed, gerium_float32_t movementSpeed, gerium_float32_t movementDelta) {
    _rotationSpeed = rotationSpeed;
    _movementSpeed = movementSpeed;
    _movementDelta = movementDelta;
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
    _pitch += deltaPitch * _rotationSpeed * delta;
    _yaw += deltaYaw * _rotationSpeed * delta;
}

void Camera::move(gerium_float32_t forward, gerium_float32_t up, gerium_float32_t right, gerium_float32_t delta) {
    glm::vec3 move{};
    move += _right * right;
    move -= _up * up;
    move += _direction * forward;
    move *= _movementSpeed * _movementDelta * delta;

    _position += move;
}

void Camera::update() {
    gerium_uint16_t width, height;
    gerium_application_get_size(_application, &width, &height);

    const auto pitch    = glm::angleAxis(_pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    const auto yaw      = glm::angleAxis(_yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    const auto rotation = glm::normalize(pitch * yaw);

    const auto R = glm::mat4_cast(rotation);
    const auto T = glm::translate(glm::identity<glm::mat4>(), _position);

    const auto aspect = float(width) / height;

    _projection     = glm::perspective(_fov, aspect, _nearPlane, _farPlane);
    _view           = R * T;
    _viewProjection = _projection * _view;

    _right     = { _view[0][0], _view[1][0], _view[2][0] };
    _up        = { _view[0][1], _view[1][1], _view[2][1] };
    _direction = { _view[0][2], _view[1][2], _view[2][2] };

    auto data            = (SceneData*) gerium_renderer_map_buffer(_renderer, _data, 0, 0);
    data->viewProjection = _viewProjection;
    data->eye            = glm::vec4(_direction, 1.0f);
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

glm::vec3 Camera::up() const noexcept {
    return _up;
}

glm::vec3 Camera::right() const noexcept {
    return _right;
}

glm::vec3 Camera::direction() const noexcept {
    return _direction;
}

gerium_descriptor_set_h Camera::getDecriptorSet() const noexcept {
    return _descriptorSet;
}
