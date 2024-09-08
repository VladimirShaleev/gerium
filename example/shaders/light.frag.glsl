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

vec3 sampleTextureCatmullRom(vec2 uv) {
    vec2 resolution = vec2(float(scene.resolution.x), float(scene.resolution.y));
    vec2 samplePosition = uv * resolution;
    vec2 texPos1 = floor(samplePosition - 0.5f) + 0.5f;

    vec2 f = samplePosition - texPos1;

    vec2 w0 = f * (-0.5f + f * (1.0f - 0.5f * f));
    vec2 w1 = 1.0f + f * f * (-2.5f + 1.5f * f);
    vec2 w2 = f * (0.5f + f * (2.0f - 1.5f * f));
    vec2 w3 = f * f * (-0.5f + 0.5f * f);

    vec2 w12 = w1 + w2;
    vec2 offset12 = w2 / (w1 + w2);

    vec2 texPos0 = texPos1 - 1;
    vec2 texPos3 = texPos1 + 2;
    vec2 texPos12 = texPos1 + offset12;

    texPos0 /= resolution;
    texPos3 /= resolution;
    texPos12 /= resolution;

    vec3 result = vec3(0);
    result += textureLod(texLight, vec2(texPos0.x, texPos0.y), 0).rgb * w0.x * w0.y;
    result += textureLod(texLight, vec2(texPos12.x, texPos0.y), 0).rgb * w12.x * w0.y;
    result += textureLod(texLight, vec2(texPos3.x, texPos0.y), 0).rgb * w3.x * w0.y;

    result += textureLod(texLight, vec2(texPos0.x, texPos12.y), 0).rgb * w0.x * w12.y;
    result += textureLod(texLight, vec2(texPos12.x, texPos12.y), 0).rgb * w12.x * w12.y;
    result += textureLod(texLight, vec2(texPos3.x, texPos12.y), 0).rgb * w3.x * w12.y;

    result += textureLod(texLight, vec2(texPos0.x, texPos3.y), 0).rgb * w0.x * w3.y;
    result += textureLod(texLight, vec2(texPos12.x, texPos3.y), 0).rgb * w12.x * w3.y;
    result += textureLod(texLight, vec2(texPos3.x, texPos3.y), 0).rgb * w3.x * w3.y;

    return result;
}

void main() {
    ivec2 pixelPosition = ivec2(
        int(scene.resolution.x * texCoord.x - 0.5), 
        int(scene.resolution.y * texCoord.y - 0.5));

    ivec2 closestPosition = ivec2(-1, -1);
    float closestDepth = 1.0;
    getClosestFragment3x3(pixelPosition, closestPosition, closestDepth);

    vec2 velocity = texelFetch(texVelocity, closestPosition, 0).rg;

    // vec4 prevLight = textureLod(texLight, texCoord - velocity, 0);
    vec4 prevLight = vec4(sampleTextureCatmullRom(texCoord - velocity), 1.0);

    vec4 light = textureLod(texAlbedo, texCoord, 0);

    outColor = mix(light, prevLight, closestPosition.x < 0 ? 0.0 : 0.9);
}
