#version 450

#include "common/types.h"

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 position;

layout(binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

void main() {
    gl_Position = scene.viewProjection * vec4(position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(1.0, 0.0, 1.0, 1.0);
}

#endif
