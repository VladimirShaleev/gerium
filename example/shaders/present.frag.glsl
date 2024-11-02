#version 450

#include "common/types.h"

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(binding = 0, set = GLOBAL_DATA_SET) uniform sampler2D texAlbedo;
layout(binding = 1, set = GLOBAL_DATA_SET) uniform sampler2D texNormal;
layout(binding = 2, set = GLOBAL_DATA_SET) uniform sampler2D texAoRoughnessMetallic;
layout(binding = 3, set = GLOBAL_DATA_SET) uniform sampler2D texMotion;

void main() {
    if (scene.settingsOutput == OUTPUT_FINAL_RESULT) {
        // Add PBR and GI
        outColor = textureLod(texAlbedo, texCoord, 0);
    } else if (scene.settingsOutput == OUTPUT_MESHLETS) {
        outColor = textureLod(texAlbedo, texCoord, 0);
    } else if (scene.settingsOutput == OUTPUT_ALBEDO) {
        outColor = textureLod(texAlbedo, texCoord, 0);
    } else if (scene.settingsOutput == OUTPUT_NORMAL) {
        outColor = textureLod(texNormal, texCoord, 0) * 0.5 + 0.5;
    } else if (scene.settingsOutput == OUTPUT_METALNESS) {
        float m = textureLod(texAoRoughnessMetallic, texCoord, 0).b;
        outColor = vec4(m, m, m, 1.0);
    } else if (scene.settingsOutput == OUTPUT_ROUGHNESS) {
        float r = textureLod(texAoRoughnessMetallic, texCoord, 0).g;
        outColor = vec4(r, r, r, 1.0);
    } else if (scene.settingsOutput == OUTPUT_MOTION) {
        outColor = vec4(textureLod(texMotion, texCoord, 0).rg * 0.5 + 0.5, 0.0, 1.0);
    }
}
