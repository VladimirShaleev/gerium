#version 450

#include "common/types.h"
#include "common/utils.h"
#include "common/lighting.h"

layout(location = 0) in vec2 texCoord;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(binding = 1, set = 1) uniform sampler2D texAlbedo;
layout(binding = 2, set = 1) uniform sampler2D texNormal;
layout(binding = 3, set = 1) uniform sampler2D texMetallicRoughness;
layout(binding = 4, set = 1) uniform sampler2D texDepth;
layout(binding = 5, set = 1) uniform sampler2D texDiffuseGI;
layout(binding = 6, set = 1) uniform sampler2D texSpecularGI;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outDiffuse;

void main() {
    vec3  albedo     = pow(textureLod(texAlbedo, texCoord, 0).rgb, vec3(2.2));
    vec3  normal     = textureLod(texNormal, texCoord, 0).rgb;
    vec3  orm        = textureLod(texMetallicRoughness, texCoord, 0).rgb;
    vec3  position   = worldPositionFromDepth(texCoord, texture(texDepth, texCoord).r, scene.invViewProjection);
    float roughness  = orm.g;
    float metallic   = orm.b;
    vec3  diffuseGI  = textureLod(texDiffuseGI, texCoord, 0).rgb;
    vec3  specularGI = textureLod(texSpecularGI, texCoord, 0).rgb;

    PixelData pexelData;
    pexelData.baseColor           = albedo;
    pexelData.F0                  = mix(vec3(0.04), albedo, metallic);;
    pexelData.F90                 = vec3(1.0);
    pexelData.uv                  = texCoord;
    pexelData.perceptualRoughness = sqrt(roughness);
    pexelData.alphaRoughness      = roughness;
    pexelData.metallic            = metallic;

    vec3 N = normalize(normal);
    vec3 V = normalize(scene.viewPosition.xyz - position);

    vec3 color = vec3(0.0);
    vec3 colorDiffuse = vec3(0.0);

    lighting(position, N, V, pexelData, diffuseGI, specularGI, color, colorDiffuse);

    outColor = vec4(color, 1.0);
    outDiffuse = vec4(colorDiffuse, 1.0);
}
