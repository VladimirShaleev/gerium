#version 460

#include "common/types.h"

layout(location = 0) in vec3 position;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(std140, binding = 0, set = MESH_DATA_SET) readonly buffer MeshDataSSBO {
    MeshData mesh[];
};

void main() {
    gl_Position = scene.viewProjection * mesh[gl_InstanceIndex].world * vec4(position, 1.0);
}
