#version 450

#include "common/types.h"
#include "common/utils.h"

layout(set = 0, binding = 0, std140) uniform VolumetricFogDataUBO {
    VolumetricFogData fogData;
};

layout(set = 1, binding = 0) uniform sampler3D noise;

layout(set = 1, binding = 1, rgba16f) uniform writeonly image3D froxelData;

layout(local_size_x = FROXEL_DISPATCH_X, local_size_y = FROXEL_DISPATCH_Y, local_size_z = FROXEL_DISPATCH_Z) in;

vec4 scatteringExtinctionFromColorDensity(vec3 color, float density) {
    const float extinction = fogData.scatteringFactor * density;
    return vec4(color * extinction, extinction);
}

vec4 unpackColorRgba(uint color) {
    return vec4((color & 0xffu) / 255.f,
                ((color >> 8u ) & 0xffu) / 255.f,
                ((color >> 16u) & 0xffu) / 255.f,
                ((color >> 24u) & 0xffu) / 255.f);
}

void main() {
    ivec3 froxelCoord = ivec3(gl_GlobalInvocationID.xyz);
    
    vec2 uv = uvFromCoords(froxelCoord.xy, 1.0 / vec2(float(fogData.froxelDimensionX), float(fogData.froxelDimensionY)));

    float depth = float(froxelCoord.z) / float(float(fogData.froxelDimensionZ));
    float linearDepth = sliceExponentialDepth(fogData.froxelNear, fogData.froxelFar, float(froxelCoord.z), float(fogData.froxelDimensionZ));

    float rawDepth = linearDepthToRawDepth(linearDepth, fogData.froxelNear, fogData.froxelFar);
    vec3 worldPosition = worldPositionFromDepth(uv, rawDepth, fogData.froxelInverseViewProjection);
    
    vec4 scatteringExtinction = vec4(0);
    
    vec3 samplingCoord = worldPosition * fogData.volumetricNoisePositionMultiplier + vec3(1.0, 0.1, 2.0) * fogData.currentFrame * fogData.volumetricNoiseSpeedMultiplier;
    float fogNoise = texture(noise, samplingCoord).r;
    fogNoise = clamp(fogNoise * fogNoise, 0.0f, 1.0f);

    float fogDensity = fogData.densityModifier * fogNoise;
    scatteringExtinction += scatteringExtinctionFromColorDensity(vec3(0.5), fogDensity);

    float heightFog = fogData.heightFogDensity * exp(-fogData.heightFogFalloff * max(worldPosition.y, 0)) * fogNoise;
    scatteringExtinction += scatteringExtinctionFromColorDensity(vec3(0.5), heightFog);

    vec3 box = abs(worldPosition - fogData.boxPosition.xyz);
    if (all(lessThanEqual(box, fogData.boxHalfSize.xyz))) {
        vec4 boxFogColor = unpackColorRgba(fogData.boxColor);
        scatteringExtinction += scatteringExtinctionFromColorDensity(boxFogColor.rgb, fogData.boxFogDensity * fogNoise);
    }

    imageStore(froxelData, froxelCoord.xyz, scatteringExtinction);
}
