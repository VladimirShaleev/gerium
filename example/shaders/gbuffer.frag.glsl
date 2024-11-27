#version 450

#extension GL_EXT_nonuniform_qualifier : enable

#include "common/types.h"

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 prevPosition;
layout(location = 2) in vec4 meshletColor;
layout(location = 3) in vec2 texcoord;
layout(location = 4) in vec3 normal;
layout(location = 5) in vec3 tangent;
layout(location = 6) in vec3 bitangent;
layout(location = 7) flat in uint instanceId;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outAoRoughnessMetallic;
layout(location = 3) out vec4 outMotion;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std430, binding = 5, set = CLUSTER_DATA_SET) readonly buffer Instances {
    Instance instances[];
};

layout(binding = 0, set = TEXTURE_SET) uniform sampler2D globalTextures[];

void main() {
    vec3 T = normalize(tangent);
    vec3 B = normalize(bitangent);
    vec3 N = normalize(normal);
    mat3 TBN = mat3(T, B, N);

    vec3 normalTex = texture(globalTextures[nonuniformEXT(instances[instanceId].normalTexture)], texcoord).rgb * 2.0 - 1.0;
    vec3 normalTBN = normalize(TBN * normalTex);

    vec4 base = texture(globalTextures[nonuniformEXT(instances[instanceId].baseTexture)], texcoord);
    vec4 mr = texture(globalTextures[nonuniformEXT(instances[instanceId].metalnessTexture)], texcoord);

    vec2 newPos = position.xy / position.w;
    vec2 oldPos = prevPosition.xy / prevPosition.w;
    newPos.xy = (newPos.xy + 1.0) * 0.5;
    oldPos.xy = (oldPos.xy + 1.0) * 0.5;
    newPos.y = 1.0 - newPos.y;
    oldPos.y = 1.0 - oldPos.y;

    outAlbedo = base;
    outNormal = vec4(normalTBN, 1.0);
    outAoRoughnessMetallic = vec4(1.0, mr.g, mr.r, 1.0);
    outMotion = vec4(newPos - oldPos, 1.0, 1.0);

    if (scene.settingsOutput == OUTPUT_MESHLETS) {
        outAlbedo = meshletColor;
    }
}
