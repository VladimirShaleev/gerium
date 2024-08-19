#version 450

#include "common/types.h"

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

layout(location = 0) out vec2 outTexcoord;

layout(binding = 0, set = 0) uniform SceneData scene;

layout(binding = 0, set = 1) uniform MeshData mesh;

void main() {
    gl_Position = scene.viewProjection * mesh.world * vec4(position, 1.0);
    outTexcoord = texcoord;
}
