#version 450

#include "common/types.h"

layout(location = 0) in vec3 position;

layout(binding = 0, set = 0) uniform SceneDataUBO {
    SceneData scene;
};

layout(binding = 0, set = 1) uniform MeshDataUBO {
    MeshData mesh;
};

void main() {
    gl_Position = scene.viewProjection * mesh.world * vec4(position, 1.0);
}
