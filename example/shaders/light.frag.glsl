#version 450

#include "common/types.h"

layout(location = 0) in vec2 texCoord;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(binding = 0, set = 1) uniform sampler2D texAlbedo;
layout(binding = 1, set = 1) uniform sampler2D texNormal;
layout(binding = 2, set = 1) uniform sampler2D texMetallicRoughness;
layout(binding = 3, set = 1) uniform sampler2D texVelocity;
layout(binding = 4, set = 1) uniform sampler2D texLight;
layout(binding = 5, set = 1) uniform sampler2D texDepth;

layout(location = 0) out vec4 outColor;

void getClosestFragment3x3(ivec2 pixel, out ivec2 closestPosition, out float closestDepth) {
    closestPosition = ivec2(-1, -1);
    closestDepth = 1.0;

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            ivec2 pixelPosition = pixel + ivec2(x, y);
            pixelPosition = clamp(pixelPosition, ivec2(0), scene.resolution - 1);
            float currentDepth = texelFetch(texDepth, pixelPosition, 0).r;
            if (currentDepth < closestDepth) {
                closestDepth = currentDepth;
                closestPosition = pixelPosition;
            }
        }
    }
}

void main() {
    ivec2 pixelPosition = ivec2(
        int(scene.resolution.x * texCoord.x - 0.5), 
        int(scene.resolution.y * texCoord.y - 0.5));

    ivec2 closestPosition = ivec2(-1, -1);
    float closestDepth = 1.0;
    getClosestFragment3x3(pixelPosition, closestPosition, closestDepth);

    vec2 velocity = texelFetch(texVelocity, closestPosition, 0).rg;
    vec4 prevLight = texture(texLight, texCoord - velocity);

    vec4 light = texture(texAlbedo, texCoord);

    outColor = mix(light, prevLight, closestPosition.x < 0 ? 0.0 : 0.9);
}
