#version 450

#include "common/types.h"

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 tangent;

layout(location = 0) out vec2 outTexcoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;
layout(location = 3) out vec3 outBitangent;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(std140, binding = 0, set = MESH_DATA_SET) uniform MeshDataUBO {
    MeshData mesh[MAX_INSTANCES];
};

void main() {
    gl_Position  = scene.viewProjection * mesh[gl_InstanceIndex].world * vec4(position, 1.0);
    outTexcoord  = texcoord;
    outNormal    = normalize(mat3(mesh[gl_InstanceIndex].inverseWorld) * normal);
    outTangent   = normalize(mat3(mesh[gl_InstanceIndex].inverseWorld) * tangent.xyz);
    outBitangent = cross(outNormal, outTangent) * tangent.w;
}
