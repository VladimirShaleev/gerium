#version 450

#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage:  require

#include "common/types.h"

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(std140, binding = 0, set = MESH_DATA_SET) readonly buffer Vertices {
    Vertex vertices[];
};

layout(location = 0) out vec4 color;

void main() {
    vec3 normal = vec3(int(vertices[gl_VertexIndex].normal.x), int(vertices[gl_VertexIndex].normal.y), int(vertices[gl_VertexIndex].normal.z)) / 127.0 - 1.0;

    gl_Position = scene.viewProjection * vertices[gl_VertexIndex].position;
    color = vec4(normal * 0.5 + vec3(0.5), 1.0);
}
