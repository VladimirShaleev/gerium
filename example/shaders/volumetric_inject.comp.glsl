#version 450

#include "common/types.h"
#include "common/utils.h"

layout(set = 0, binding = 0, std140) uniform VolumetricFogDataUBO {
    VolumetricFogData fog;
};

layout(set = 1, binding = 0) uniform sampler3D noise;

layout(set = 1, binding = 1) uniform sampler2D blueNoise;

layout(set = 1, binding = 2) uniform sampler2DArray csm;

layout(set = 1, binding = 3) uniform sampler3D prevIntegratedLight;

layout(set = 1, binding = 4, rgba16f) uniform writeonly image3D froxelData;

layout(std140, set = 2, binding = 0) uniform LightSpaceMatricesUBO {
    mat4 lightSpaceMatrices[CSM_MAX_CASCADES];
};

layout(std140, set = 2, binding = 1) uniform CascadeDistancesUBO {
    float cascadeDistances[CSM_MAX_CASCADES];
};

layout(local_size_x = FROXEL_LOCAL_X, local_size_y = FROXEL_LOCAL_Y, local_size_z = FROXEL_LOCAL_Z) in;

float visibility(vec3 worldPosition) {
    float depth = (fog.view * vec4(worldPosition, 1.0)).z;

    int layer = 0;
    for (int i = 0; i < CSM_MAX_CASCADES - 1; ++i) {
        if (depth < cascadeDistances[i]) {
            break;
        }
        layer = i + 1;
    }

    vec4 lightSpacePos = lightSpaceMatrices[layer] * vec4(worldPosition, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = 1.0 - (projCoords.y * 0.5 + 0.5);

    if (any(greaterThan(projCoords, vec3(1.0))) || any(lessThan(projCoords, vec3(0.0)))) {
        return 1.0;
    }

    float bias = fog.biasNearFarPow.x;
    bias *= 1.0 / (cascadeDistances[layer] * 0.5);

    float pcfDepth = texture(csm, vec3(projCoords.xy, layer)).r; 
    float shadow = pcfDepth < projCoords.z + bias ? 0.0 : 1.0;   
    return shadow;
}

float sampleBlueNoise(ivec3 coord) {
    ivec2 noiseCoord = (coord.xy + ivec2(0, 1) * coord.z * 128) % 128;
    return texelFetch(blueNoise, noiseCoord, 0).r;
}

float phaseFunction(vec3 wo, vec3 wi, float g) {
    float cosTheta = dot(wo, wi);
    float denom     = 1.0 + g * g - 2.0 * g * cosTheta;
    return (1.0 / (4.0 * PI)) * (1.0 - g * g) / max(pow(denom, 1.5f), 0.0001);
}

void main() {
    ivec3 coord = ivec3(gl_GlobalInvocationID.xyz);
    
    if (all(lessThan(coord, ivec3(FROXEL_GRID_SIZE_X, FROXEL_GRID_SIZE_Y, FROXEL_GRID_SIZE_Z)))) {
        float jitter = (sampleBlueNoise(coord) - 0.5) * 0.999;

        vec3 worldPos = coordToWorldWithJitter(coord, jitter, fog.biasNearFarPow.y, fog.biasNearFarPow.z, fog.invViewProj);
            
        // vec3 worldPos = worldPositionFromFroxel(
        //     coord, 
        //     uvec3(FROXEL_GRID_SIZE_X, FROXEL_GRID_SIZE_Y, FROXEL_GRID_SIZE_Z), 
        //     fog.biasNearFarPow.y, 
        //     fog.biasNearFarPow.z, 
        //     fog.invViewProj,
        //     vec2(0.0));

        vec3 V = normalize(fog.cameraPosition.xyz - worldPos);

        float density = fog.anisoDensityScatteringAbsorption.y;

        vec3 lighting = fog.lightColor.rgb * fog.lightColor.a;
        
        float visibilityValue = visibility(worldPos);
        
        if (visibilityValue > 0.0001) {
            lighting += visibilityValue * fog.lightColor.rgb * phaseFunction(V, -fog.lightDirection.xyz, fog.anisoDensityScatteringAbsorption.x);
        }

        vec4 colorAndDensity = vec4(lighting * density, density);

        vec3 worldPosWithoutJitter = coordToWorldWithJitter(coord, 0.0, fog.biasNearFarPow.y, fog.biasNearFarPow.z, fog.invViewProj);

        vec3 historyUv = worldToUv(worldPosWithoutJitter, fog.biasNearFarPow.y, fog.biasNearFarPow.z, fog.prevViewProj);

        if (all(greaterThanEqual(historyUv, vec3(0.0))) && all(lessThanEqual(historyUv, vec3(1.0)))) {
            vec4 history = textureLod(prevIntegratedLight, historyUv, 0.0);

            colorAndDensity = mix(history, colorAndDensity, 0.05);
        }

        imageStore(froxelData, coord, colorAndDensity);
    }
}
