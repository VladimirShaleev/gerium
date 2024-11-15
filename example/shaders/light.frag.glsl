#version 450

#include "common/defines.h"

#include "common/types.h"
#include "common/utils.h"

layout(location = 0) in vec2 texCoord;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(binding = 0, set = 1) uniform sampler2D texAlbedo;
layout(binding = 1, set = 1) uniform sampler2D texNormal;
layout(binding = 2, set = 1) uniform sampler2D texMetallicRoughness;
layout(binding = 3, set = 1) uniform sampler2D texDepth;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 albedo = pow(textureLod(texAlbedo, texCoord, 0).rgb, vec3(2.2));
    vec3 normal = textureLod(texNormal, texCoord, 0).rgb;
    vec3 orm = textureLod(texMetallicRoughness, texCoord, 0).rgb;
    vec3 position = worldPositionFromDepth(texCoord, texture(texDepth, texCoord).r, scene.invViewProjection);
    float ao = orm.r * 0.2;
    float roughness = orm.g;
    float metallic = orm.b;

    PixelData pexelData;
    pexelData.baseColor           = albedo;
    pexelData.F0                  = mix(vec3(0.04), albedo, metallic);;
    pexelData.F90                 = vec3(1.0);
    pexelData.perceptualRoughness = sqrt(roughness);
    pexelData.alphaRoughness      = roughness;
    pexelData.metallic            = metallic;

    vec3 N = normalize(normal);
    vec3 V = normalize(scene.viewPosition.xyz - position);

    vec3 color = vec3(0.0);
    vec3 colorDiffuse = vec3(0.0);

    lighting(position, N, V, pexelData, color, colorDiffuse);

    outColor = vec4(color, 1.0);
}
