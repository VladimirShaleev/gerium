#version 450

#extension GL_EXT_nonuniform_qualifier : enable

#include "common/types.h"

layout(location = 0) in vec4 meshletColor;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) flat in uint instanceId;

layout(location = 0) out vec4 outColor;

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
    // T = normalize(T - dot(T, N) * N);
    mat3 TBN = mat3(T, B, N);

    vec3 normalTex = texture(globalTextures[nonuniformEXT(instances[instanceId].normalTexture)], texcoord).rgb * 2.0 - 1.0;
    vec3 normalTBN = normalize(TBN * normalTex);

    vec4 base = texture(globalTextures[nonuniformEXT(instances[instanceId].baseTexture)], texcoord);
    vec4 mr = texture(globalTextures[nonuniformEXT(instances[instanceId].metalnessTexture)], texcoord);

    if (scene.settingsOutput == OUTPUT_FINAL_RESULT) {
        // Add PBR and GI
        outColor = base;
    } else if (scene.settingsOutput == OUTPUT_MESHLETS) {
        outColor = meshletColor;
    } else if (scene.settingsOutput == OUTPUT_ALBEDO) {
        outColor = base;
    } else if (scene.settingsOutput == OUTPUT_NORMAL) {
        outColor = vec4(normalTBN * 0.5 + 0.5, 1.0);
    } else if (scene.settingsOutput == OUTPUT_METALNESS) {
        outColor = vec4(mr.r, mr.r, mr.r, 1.0);
    } else if (scene.settingsOutput == OUTPUT_ROUGHNESS) {
        outColor = vec4(mr.g, mr.g, mr.g, 1.0);
    }
}
