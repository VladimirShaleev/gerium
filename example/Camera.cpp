#include "Camera.hpp"

Camera::Camera(gerium_application_t application, ResourceManager& resourceManager) noexcept :
    _application(application),
    _renderer(resourceManager.renderer()),
    _resourceManager(&resourceManager) {
    setSpeed();
    setPosition({ 0.0f, 0.0f, 0.0f });
    setRotation(0.0f, 0.0f);
    setPerpective(0.01f, 1000.0f, glm::radians(60.0f));
}

Camera::Camera(const Camera& other) {
    copy(other);
}

Camera& Camera::operator=(const Camera& other) {
    if (this != &other) {
        copy(other);
    }
    return *this;
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

void Camera::jittering(gerium_float32_t dx, gerium_float32_t dy) {
    _prevJitter = _jitter;
    _jitter.x   = dx;
    _jitter.y   = dy;
}

void Camera::update() {
    if (_active == nullptr) {
        _active = this;
    }
    if (!_descriptorSet) {
        _descriptorSet = _resourceManager->createDescriptorSet("");
    }
    if (!_data) {
        gerium_descriptor_set_h ds = _descriptorSet;
        _data                      = _resourceManager->createBuffer(GERIUM_BUFFER_USAGE_UNIFORM_BIT,
                                               true,
                                               "scene_data_" + std::to_string(ds.index),
                                               nullptr,
                                               sizeof(SceneData));
        gerium_renderer_bind_buffer(_renderer, _descriptorSet, 0, _data);
    }

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

    const auto aspect = float(width) / height;

    auto jitter         = glm::translate(glm::vec3(_jitter, 0.0f));
    _prevViewProjection = _viewProjection;
    _projection         = jitter * infinitePerspectiveReverse(_fov, aspect, _nearPlane);
    _view               = glm::lookAt(_position, _position + _front, _up);
    _viewProjection     = _projection * _view;

    _frustum[LeftFace].normal.x = _viewProjection[0][3] + _viewProjection[0][0];
    _frustum[LeftFace].normal.y = _viewProjection[1][3] + _viewProjection[1][0];
    _frustum[LeftFace].normal.z = _viewProjection[2][3] + _viewProjection[2][0];
    _frustum[LeftFace].distance = _viewProjection[3][3] + _viewProjection[3][0];
    _frustum[LeftFace].normalize();

    _frustum[RightFace].normal.x = _viewProjection[0][3] - _viewProjection[0][0];
    _frustum[RightFace].normal.y = _viewProjection[1][3] - _viewProjection[1][0];
    _frustum[RightFace].normal.z = _viewProjection[2][3] - _viewProjection[2][0];
    _frustum[RightFace].distance = _viewProjection[3][3] - _viewProjection[3][0];
    _frustum[RightFace].normalize();

    _frustum[BottomFace].normal.x = _viewProjection[0][3] + _viewProjection[0][1];
    _frustum[BottomFace].normal.y = _viewProjection[1][3] + _viewProjection[1][1];
    _frustum[BottomFace].normal.z = _viewProjection[2][3] + _viewProjection[2][1];
    _frustum[BottomFace].distance = _viewProjection[3][3] + _viewProjection[3][1];
    _frustum[BottomFace].normalize();

    _frustum[TopFace].normal.x = _viewProjection[0][3] - _viewProjection[0][1];
    _frustum[TopFace].normal.y = _viewProjection[1][3] - _viewProjection[1][1];
    _frustum[TopFace].normal.z = _viewProjection[2][3] - _viewProjection[2][1];
    _frustum[TopFace].distance = _viewProjection[3][3] - _viewProjection[3][1];
    _frustum[TopFace].normalize();

    _frustum[NearFace].normal.x = _viewProjection[0][2];
    _frustum[NearFace].normal.y = _viewProjection[1][2];
    _frustum[NearFace].normal.z = _viewProjection[2][2];
    _frustum[NearFace].distance = _viewProjection[3][2];
    _frustum[NearFace].normalize();

    _frustum[FarFace].normal.x = _viewProjection[0][3] - _viewProjection[0][2];
    _frustum[FarFace].normal.y = _viewProjection[1][3] - _viewProjection[1][2];
    _frustum[FarFace].normal.z = _viewProjection[2][3] - _viewProjection[2][2];
    _frustum[FarFace].distance = _viewProjection[3][3] - _viewProjection[3][2];
    _frustum[FarFace].normalize();

    auto projectionT = glm::transpose(_projection);

    vec4 frustumX = projectionT[3] + projectionT[0];
    vec4 frustumY = projectionT[3] + projectionT[1];
    frustumX /= glm::length(frustumX.xyz());
    frustumY /= glm::length(frustumY.xyz());

    _pyramidResolution = glm::uvec2(previousPow2(width), previousPow2(height));

    _sceneData.view               = _view;
    _sceneData.viewProjection     = _viewProjection;
    _sceneData.prevViewProjection = _prevViewProjection;
    _sceneData.invViewProjection  = glm::inverse(_viewProjection);
    _sceneData.viewPosition       = glm::vec4(_position, 1.0f);
    _sceneData.eye                = glm::vec4(_front, 1.0f);
    _sceneData.frustum            = vec4(frustumX.x, frustumX.z, frustumY.y, frustumY.z);
    _sceneData.p00p11             = glm::vec2(_projection[0][0], _projection[1][1]);
    _sceneData.farNear            = glm::vec2(_farPlane, _nearPlane);
    _sceneData.jitter             = _jitter;
    _sceneData.prevJitter         = _prevJitter;
    _sceneData.invResolution      = { 1.0f / width, 1.0f / height };
    _sceneData.resolution         = { width, height };
    _sceneData.pyramidResolution  = { float(_pyramidResolution.x), float(_pyramidResolution.y) };
    _sceneData.lodTarget          = (2.0f / _sceneData.p00p11.y) * _sceneData.invResolution.x;

    auto ptr = (SceneData*) gerium_renderer_map_buffer(_renderer, _data, 0, 0);
    *ptr     = _sceneData;
    gerium_renderer_unmap_buffer(_renderer, _data);
}

Intersection Camera::test(const glm::vec3& point) const noexcept {
    bool onPlane = false;
    for (int i = 0; i < 6; ++i) {
        if (_frustum[i].getDistanceToPlane(point) < 0.0f) {
            return Intersection::None;
        }
    }
    return Intersection::Full;
}

Intersection Camera::test(const BoundingBox& bbox) const noexcept {
    const glm::vec3& p1 = bbox.min();
    const glm::vec3& p2 = bbox.max();

    const glm::vec3 points[] = { glm::vec3(p1.x, p1.y, p1.z), glm::vec3(p2.x, p1.y, p1.z), glm::vec3(p2.x, p1.y, p2.z),
                                 glm::vec3(p1.x, p1.y, p2.z), glm::vec3(p1.x, p2.y, p1.z), glm::vec3(p2.x, p2.y, p1.z),
                                 glm::vec3(p2.x, p2.y, p2.z), glm::vec3(p1.x, p2.y, p2.z) };

    int total = 0;

    for (int i = 0; i < 6; ++i) {
        int count    = 8;
        bool pointIn = true;

        for (auto& point : points) {
            if (_frustum[i].getDistanceToPlane(point) < 0.0f) {
                pointIn = false;
                --count;
            }
        }

        if (count == 0) {
            return Intersection::None;
        }

        total += pointIn ? 1 : 0;
    }

    return total == 6 ? Intersection::Full : Intersection::Partial;
}

bool Camera::isActive() const noexcept {
    return _active == this;
}

void Camera::activate() noexcept {
    _active = this;
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

const SceneData& Camera::sceneData() const noexcept {
    return _sceneData;
}

const DescriptorSet& Camera::getDecriptorSet() const noexcept {
    return _descriptorSet;
}

glm::vec2 Camera::calcJitter(Jitter jitter, gerium_sint32_t index, gerium_sint32_t jitterPeriod) noexcept {
    switch (jitter) {
        case Jitter::Halton:
            return halton23(index);
        case Jitter::R2:
            return mRobertR2(index);
        case Jitter::Hammersley:
            return hammersley(index, jitterPeriod);
        case Jitter::InterleavedGradients:
            return interleavedGradient(index);
        default:
            assert(!"unreachable code");
            return {};
    }
}

std::vector<glm::vec2> Camera::calcJitterTable(Jitter jitter, gerium_sint32_t jitterPeriod) noexcept {
    std::vector<glm::vec2> table;
    table.resize(jitterPeriod);
    for (int i = 0; i < jitterPeriod; ++i) {
        table[i] = calcJitter(jitter, i, jitterPeriod);
    }
    return table;
}

glm::mat4 Camera::infinitePerspectiveReverse(gerium_float32_t fov,
                                             gerium_float32_t aspect,
                                             gerium_float32_t nearPlane) noexcept {
    const auto f = 1.0f / glm::tan(fov / 2.0f);
    auto result  = glm::zero<glm::mat4>();
    result[0][0] = f / aspect;
    result[1][1] = f;
    result[2][2] = 0.0f;
    result[2][3] = 1.0f;
    result[3][2] = nearPlane;
    return result;
}

void Camera::copy(const Camera& other) noexcept {
    _application     = other._application;
    _renderer        = other._renderer;
    _resourceManager = other._resourceManager;
    _position        = other._position;
    _front           = other._front;
    _up              = other._up;
    _right           = other._right;
    _yaw             = other._yaw;
    _pitch           = other._pitch;
    _movementSpeed   = other._movementSpeed;
    _rotationSpeed   = other._rotationSpeed;
    _nearPlane       = other._nearPlane;
    _farPlane        = other._farPlane;
    _fov             = other._fov;
    _view            = other._view;
    _projection      = other._projection;
    _viewProjection  = other._viewProjection;
}

Camera* Camera::_active{};
