#version 450

#include "common/types.h"

layout(location = 0) in vec4 position;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std430, binding = 5, set = GLOBAL_DATA_SET) readonly buffer Instances {
    Instance instances[];
};

void main() {
    gl_Position = scene.viewProjection * instances[gl_InstanceIndex].world * position;
}
