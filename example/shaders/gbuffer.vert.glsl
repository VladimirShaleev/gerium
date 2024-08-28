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

layout(std140, binding = 0, set = 0) uniform SceneDataUBO {
    SceneData scene;
};

layout(std140, binding = 0, set = 1) uniform MeshDataUBO {
    MeshData mesh;
};

void main() {
    gl_Position  = scene.viewProjection * mesh.world * vec4(position, 1.0);
    outTexcoord  = texcoord;
    outNormal    = normalize(mat3(mesh.inverseWorld) * normal);
    outTangent   = normalize(mat3(mesh.inverseWorld) * tangent.xyz);
    outBitangent = cross(outNormal, outTangent) * tangent.w;
}
