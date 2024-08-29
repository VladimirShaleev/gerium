#version 450

#include "common/types.h"

layout(location = 0) in vec2 inTexcoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMetallicRoughness;

layout(binding = 0, set = TEXTURE_SET) uniform sampler2D baseColor;
layout(binding = 1, set = TEXTURE_SET) uniform sampler2D normalColor;
layout(binding = 2, set = TEXTURE_SET) uniform sampler2D metallicRoughnessColor;

void main() {
    vec3 T = normalize(inTangent);
    vec3 B = normalize(inBitangent);
    vec3 N = normalize(inNormal);
    T = normalize(T - dot(T, N) * N);
    mat3 TBN = mat3(T, B, N);
    vec3 bumpNormal = 2.0 * texture(normalColor, inTexcoord).rgb - vec3(1.0);
    vec3 normal = normalize(TBN * bumpNormal);

    vec4 rm = texture(metallicRoughnessColor, inTexcoord);
    float occlusion = rm.r;
    float roughness = rm.g;
    float metalness = rm.b;

    outColor = texture(baseColor, inTexcoord);
    outNormal = vec4(normal, 0.0);
    outMetallicRoughness = vec4(occlusion, roughness, metalness, 0.0);
}
