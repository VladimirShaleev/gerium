#version 450

#include "common/types.h"
#include "common/textures.h"

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;
layout(location = 4) in vec4 inPosition;
layout(location = 5) in vec4 inPrevPosition;
layout(location = 6) flat in int inInstanceID; 

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMetallicRoughness;
layout(location = 3) out vec2 outVelocity;

layout(std140, binding = 0, set = MESH_DATA_SET) readonly buffer MeshDataSSBO {
    MeshData mesh[];
};

definePbrTextures()

void main() {
    vec3 T = normalize(inTangent);
    vec3 B = normalize(inBitangent);
    vec3 N = normalize(inNormal);
    T = normalize(T - dot(T, N) * N);
    mat3 TBN = mat3(T, B, N);
    vec3 bumpNormal = 2.0 * fetchNormal(inInstanceID, inTexcoord).rgb - vec3(1.0);
    vec3 normal = normalize(TBN * bumpNormal);

    vec4 rm = fetchMetallicRoughness(inInstanceID, inTexcoord);
    float occlusion = rm.r;
    float roughness = rm.g;
    float metalness = rm.b;

    outColor = fetchBase(inInstanceID, inTexcoord);
    outNormal = vec4(normal, 0.0);
    outMetallicRoughness = vec4(occlusion, roughness, metalness, 0.0);

    vec2 newPos = inPosition.xy / inPosition.w;
    newPos.xy = (newPos.xy + 1.0) / 2.0;
    newPos.y = 1.0 - newPos.y;

    vec2 oldPos = inPrevPosition.xy / inPrevPosition.w;
    oldPos.xy = (oldPos.xy + 1.0) / 2.0;
    oldPos.y = 1.0 - oldPos.y;

    outVelocity = newPos - oldPos;
}
