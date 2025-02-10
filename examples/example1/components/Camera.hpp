#ifndef COMPONENTS_CAMERA_HPP
#define COMPONENTS_CAMERA_HPP

#include "../Common.hpp"

struct Camera {
    bool active{};

    glm::vec3 position{};
    glm::vec3 front{};
    glm::vec3 up{};
    glm::vec3 right{};

    gerium_float32_t yaw{};
    gerium_float32_t pitch{};

    gerium_float32_t movementSpeed{};
    gerium_float32_t rotationSpeed{};

    gerium_float32_t nearPlane{};
    gerium_float32_t farPlane{};
    gerium_float32_t fov{};

    glm::ivec2 resolution{};

    glm::mat4 view{};
    glm::mat4 prevView{};
    glm::mat4 projection{};
    glm::mat4 prevProjection{};
    glm::mat4 viewProjection{};
    glm::mat4 prevViewProjection{};
};

#endif
