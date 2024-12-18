#version 450

#include "common/types.h"
#include "common/utils.h"
#include "common/lighting.h"

layout(location = 0) in vec2 texCoord;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(std140, binding = 1, set = SCENE_DATA_SET) uniform VolumetricFogDataUBO {
    VolumetricFogData fog;
};

layout(binding = 0, set = 1) uniform sampler2D texAlbedo;
layout(binding = 1, set = 1) uniform sampler2D texNormal;
layout(binding = 2, set = 1) uniform sampler2D texMetallicRoughness;
layout(binding = 3, set = 1) uniform sampler2D texAO;
layout(binding = 4, set = 1) uniform sampler2D texDepth;
layout(binding = 5, set = 1) uniform sampler2D texDiffuseGI;
layout(binding = 6, set = 1) uniform sampler2D texSpecularGI;
layout(binding = 7, set = 1) uniform sampler3D texIntegratedLightScattering;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outDiffuse;

// void calcVolumetricFog(vec2 uv, float rawDepth, out vec3 color, out float a) {
//     const float near = fog.froxelNear;
//     const float far = fog.froxelFar;
//     
//     float linearDepth = rawDepthToLinearDepth(rawDepth, near, far);
//     
//     float depthUv = linearDepthToUv(near, far, linearDepth, fog.froxelDimensionZ);
//     vec3 froxelUvw = vec3(uv, depthUv);
//     vec4 scatteringTransmittance = texture(texIntegratedLightScattering, froxelUvw);
// 
//     const float scatteringModifier = max(1 - scatteringTransmittance.a, 0.00000001);
// 
//     color = scatteringTransmittance.rgb * scatteringModifier;
//     a = scatteringTransmittance.a;
// }

// https://gist.github.com/Fewes/59d2c831672040452aa77da6eaab2234
vec4 textureTricubic(vec3 coord) {
    vec3 tsize = vec3(textureSize(texIntegratedLightScattering, 0));
	vec3 coordGrid = coord * tsize - 0.5;
	vec3 index = floor(coordGrid);
	vec3 fraction = coordGrid - index;
	vec3 oneFrac = 1.0 - fraction;

	vec3 w0 = 1.0 / 6.0 * oneFrac * oneFrac * oneFrac;
	vec3 w1 = 2.0 / 3.0 - 0.5 * fraction * fraction * (2.0 - fraction);
	vec3 w2 = 2.0 / 3.0 - 0.5 * oneFrac * oneFrac * (2.0 - oneFrac);
	vec3 w3 = 1.0 / 6.0 * fraction * fraction * fraction;

	vec3 g0 = w0 + w1;
	vec3 g1 = w2 + w3;
	vec3 mult = 1.0 / tsize;
	vec3 h0 = mult * ((w1 / g0) - 0.5 + index); //h0 = w1/g0 - 1, move from [-0.5, tsize-0.5] to [0,1]
	vec3 h1 = mult * ((w3 / g1) + 1.5 + index); //h1 = w3/g1 + 1, move from [-0.5, tsize-0.5] to [0,1]

	// Fetch the eight linear interpolations
	// Weighting and fetching is interleaved for performance and stability reasons
	vec4 tex000 = texture(texIntegratedLightScattering, h0, 0.0);
	vec4 tex100 = texture(texIntegratedLightScattering, vec3(h1.x, h0.y, h0.z), 0.0f);
	tex000 = mix(tex100, tex000, g0.x); // Weigh along the x-direction

	vec4 tex010 = texture(texIntegratedLightScattering, vec3(h0.x, h1.y, h0.z), 0.0f);
	vec4 tex110 = texture(texIntegratedLightScattering, vec3(h1.x, h1.y, h0.z), 0.0f);
	tex010 = mix(tex110, tex010, g0.x); // Weigh along the x-direction
	tex000 = mix(tex010, tex000, g0.y); // Weigh along the y-direction

	vec4 tex001 = texture(texIntegratedLightScattering, vec3(h0.x, h0.y, h1.z), 0.0f);
	vec4 tex101 = texture(texIntegratedLightScattering, vec3(h1.x, h0.y, h1.z), 0.0f);
	tex001 = mix(tex101, tex001, g0.x); // Weigh along the x-direction

	vec4 tex011 = texture(texIntegratedLightScattering, vec3(h0.x, h1.y, h1.z), 0.0f);
	vec4 tex111 = texture(texIntegratedLightScattering, vec3(h1), 0.0f);
	tex011 = mix(tex111, tex011, g0.x); // Weigh along the x-direction
	tex001 = mix(tex011, tex001, g0.y); // Weigh along the y-direction

	return mix(tex001, tex000, g0.z); // Weigh along the z-direction
}

void calcVolumetricFog(vec3 worldPos, out vec3 color, out float a) {
    vec3 froxelUvw = worldToUv(worldPos, fog.biasNearFarPow.y, fog.biasNearFarPow.z, fog.viewProj);

    vec4  scatteredLight = textureTricubic(froxelUvw);
    float transmittance  = scatteredLight.a;

    a = transmittance;
    color = scatteredLight.rgb;
}

void main() {
    vec3  albedo     = pow(textureLod(texAlbedo, texCoord, 0).rgb, vec3(2.2));
    vec3  normal     = textureLod(texNormal, texCoord, 0).rgb;
    vec3  orm        = vec3(textureLod(texAO, texCoord, 0).r, textureLod(texMetallicRoughness, texCoord, 0).gb);
    vec3  position   = worldPositionFromDepth(texCoord, texture(texDepth, texCoord).r, scene.invViewProjection);
    vec4  viewPos    = scene.view * vec4(position, 1.0);
    float ao         = orm.r;
    float roughness  = orm.g;
    float metallic   = orm.b;
    vec3  diffuseGI  = textureLod(texDiffuseGI, texCoord, 0).rgb;
    vec3  specularGI = textureLod(texSpecularGI, texCoord, 0).rgb;
    
    vec3 froxelColor;
    float froxelA;
    calcVolumetricFog(position, froxelColor, froxelA);

    PixelData pixelData;
    pixelData.baseColor           = albedo;
    pixelData.F0                  = mix(vec3(0.04), albedo, metallic);;
    pixelData.F90                 = vec3(1.0);
    pixelData.uv                  = texCoord;
    pixelData.perceptualRoughness = sqrt(roughness);
    pixelData.alphaRoughness      = roughness;
    pixelData.metallic            = metallic;
    pixelData.ambientOcclusion    = ao;

    vec3 N = normalize(normal);
    vec3 V = normalize(scene.viewPosition.xyz - position);

    vec3 color = vec3(0.0);
    vec3 colorDiffuse = vec3(0.0);

    bool directLightOnly = scene.settingsOutput == OUTPUT_DIRECT_LIGHT_ONLY;

    lighting(position, viewPos.z, N, V, pixelData, diffuseGI, specularGI, directLightOnly, color, colorDiffuse);

    outColor = vec4(color, 1.0);
    outDiffuse = vec4(colorDiffuse, 1.0);

    outColor.rgb = outColor.rgb * froxelA + froxelColor;
    outDiffuse.rgb = outDiffuse.rgb * froxelA + froxelColor;
}
