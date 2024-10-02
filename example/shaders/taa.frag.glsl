#version 450

#include "common/types.h"
#include "common/utils.h"

layout(location = 0) in vec2 texCoord;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(binding = 0, set = 1) uniform sampler2D texCurrent;
layout(binding = 1, set = 1) uniform sampler2D texPrevious;
layout(binding = 2, set = 1) uniform sampler2D texVelocity;
layout(binding = 3, set = 1) uniform sampler2D texDepth;

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
    vec2 texPos1 = floor(samplePosition - 0.5) + 0.5;

    vec2 f = samplePosition - texPos1;

    vec2 w0 = f * (-0.5 + f * (1.0 - 0.5 * f));
    vec2 w1 = 1.0 + f * f * (-2.5 + 1.5 * f);
    vec2 w2 = f * (0.5 + f * (2.0 - 1.5 * f));
    vec2 w3 = f * f * (-0.5 + 0.5 * f);

    vec2 w12 = w1 + w2;
    vec2 offset12 = w2 / (w1 + w2);

    vec2 texPos0 = texPos1 - 1;
    vec2 texPos3 = texPos1 + 2;
    vec2 texPos12 = texPos1 + offset12;

    texPos0 /= resolution;
    texPos3 /= resolution;
    texPos12 /= resolution;

    vec3 result = vec3(0);
    result += textureLod(texPrevious, vec2(texPos0.x, texPos0.y), 0).rgb * w0.x * w0.y;
    result += textureLod(texPrevious, vec2(texPos12.x, texPos0.y), 0).rgb * w12.x * w0.y;
    result += textureLod(texPrevious, vec2(texPos3.x, texPos0.y), 0).rgb * w3.x * w0.y;

    result += textureLod(texPrevious, vec2(texPos0.x, texPos12.y), 0).rgb * w0.x * w12.y;
    result += textureLod(texPrevious, vec2(texPos12.x, texPos12.y), 0).rgb * w12.x * w12.y;
    result += textureLod(texPrevious, vec2(texPos3.x, texPos12.y), 0).rgb * w3.x * w12.y;

    result += textureLod(texPrevious, vec2(texPos0.x, texPos3.y), 0).rgb * w0.x * w3.y;
    result += textureLod(texPrevious, vec2(texPos12.x, texPos3.y), 0).rgb * w12.x * w3.y;
    result += textureLod(texPrevious, vec2(texPos3.x, texPos3.y), 0).rgb * w3.x * w3.y;

    return result;
}

vec3 accumulateSampleAndWeights(ivec2 pos, out vec3 neighborhoodMin, out vec3 neighborhoodMax, out vec3 m1, out vec3 m2) {
    float currentSampleWeight = 0.0;
    vec3 currentSampleTotal = vec3(0.0);
    neighborhoodMin = vec3(10000.0);
    neighborhoodMax = vec3(-10000.0);
    m1 = vec3(0.0);
    m2 = vec3(0.0);

    for (int x = -1; x <= 1; ++x ) {
        for (int y = -1; y <= 1; ++y ) {
            ivec2 pixelPosition = pos + ivec2(x, y);
            pixelPosition = clamp(pixelPosition, ivec2(0), scene.resolution - 1);

            vec3 currentSample = texelFetch(texCurrent, pixelPosition, 0).rgb;
            vec2 subsamplePosition = vec2(x * 1.0, y * 1.0);
            float subsampleDistance = length(subsamplePosition);
            float subsampleWeight = subsampleFilter(subsampleDistance);

            currentSampleTotal += currentSample * subsampleWeight;
            currentSampleWeight += subsampleWeight;

            neighborhoodMin = min(neighborhoodMin, currentSample);
            neighborhoodMax = max(neighborhoodMax, currentSample);

            m1 += currentSample;
            m2 += currentSample * currentSample;
        }
    }

    return currentSampleTotal / currentSampleWeight;
}

void main() {
    ivec2 pixelPosition = ivec2(
        int(floor((scene.resolution.x - 1) * texCoord.x)), 
        int(floor((scene.resolution.y - 1) * texCoord.y)));

    ivec2 closestPosition = ivec2(-1, -1);
    float closestDepth = 1.0;
    getClosestFragment3x3(pixelPosition, closestPosition, closestDepth);

    vec2 velocity = texelFetch(texVelocity, closestPosition, 0).rg;
    vec2 reprojectedUV = texCoord - velocity;

    vec3 historyColor = sampleTextureCatmullRom(reprojectedUV);

    vec3 neighborhoodMin;
    vec3 neighborhoodMax;
    vec3 m1;
    vec3 m2;
    vec3 currentSample = accumulateSampleAndWeights(pixelPosition, neighborhoodMin, neighborhoodMax, m1, m2);
    if (any(lessThan(reprojectedUV, vec2(0.0))) || any(greaterThan(reprojectedUV, vec2(1.0)))) {
        outColor = vec4(currentSample, 1.0);
        return;
    }

    historyColor = varianceClipClamp(historyColor.rgb, neighborhoodMin, neighborhoodMax, m1, m2);

    vec3 result = mix(currentSample, historyColor, 0.9);
    outColor = vec4(result, 1.0);
}
