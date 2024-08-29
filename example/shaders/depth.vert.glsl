#version 450

#include "common/types.h"

layout(location = 0) in vec3 position;

layout(binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(binding = 0, set = MESH_DATA_SET) uniform MeshDataUBO {
    MeshData mesh[MAX_INSTANCES];
};

void main() {
    gl_Position = scene.viewProjection * mesh[gl_InstanceIndex].world * vec4(position, 1.0);
}
