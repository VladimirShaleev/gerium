#version 450

#extension GL_EXT_nonuniform_qualifier : enable

#include "common/types.h"

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) flat in uint instanceId;

layout(location = 0) out vec4 outColor;

layout(std430, binding = 5, set = CLUSTER_DATA_SET) readonly buffer Instances {
    Instance instances[];
};

layout(binding = 0, set = TEXTURE_SET) uniform sampler2D globalTextures[];

void main() {
    vec3 T = normalize(tangent);
    vec3 B = normalize(bitangent);
    vec3 N = normalize(normal);
    T = normalize(T - dot(T, N) * N);
    mat3 TBN = mat3(T, B, N);

    vec3 normalTex = texture(globalTextures[nonuniformEXT(0)], texcoord).rgb * 2.0 - 1.0;
    vec3 normalTBN = normalize(TBN * normalTex);

    outColor = texture(globalTextures[nonuniformEXT(instances[instanceId].baseTexture)], texcoord);
    // outColor.r = normalTBN.r * 0.5 + 0.5;
    // outColor.g = normalTBN.g * 0.5 + 0.5;
    // outColor.b = normalTBN.b * 0.5 + 0.5;
}
