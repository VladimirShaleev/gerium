#version 450

#include "common/types.h"

#ifdef VERTEX

layout(location = 0) in vec3 position;

layout(binding = 0, set = 0) uniform SceneDataUBO {
    SceneData scene;
};

void main() {
    gl_Position = scene.viewProjection * vec4(position, 1.0);
}

#endif

#ifdef FRAGMENT

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0, 0.0, 1.0, 1.0);
}

#endif
