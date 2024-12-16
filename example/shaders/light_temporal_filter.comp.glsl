#version 450

#include "common/types.h"
#include "common/utils.h"

layout(set = 0, binding = 0, std140) uniform VolumetricFogDataUBO {
    VolumetricFogData fog;
};

layout(set = 1, binding = 0) uniform sampler3D lightScattering;

layout(set = 1, binding = 1) uniform sampler3D prevLightScattering;

layout(set = 1, binding = 2, rgba16f) uniform writeonly image3D lightScatteringFiltering;

layout(local_size_x = FROXEL_DISPATCH_X, local_size_y = FROXEL_DISPATCH_Y, local_size_z = FROXEL_DISPATCH_Z) in;

void main() {
    ivec3 froxelCoord = ivec3(gl_GlobalInvocationID.xyz);
    
    vec3 rcpFroxelDim = 1.0 / vec3(fog.froxelDimensionX, fog.froxelDimensionY, fog.froxelDimensionZ);

    vec4 scatteringExtinction = texture(lightScattering, froxelCoord * rcpFroxelDim);

    // vec3 worldPosition = worldPositionFromFroxel(
    //     froxelCoord, 
    //     uvec3(fog.froxelDimensionX, fog.froxelDimensionY, fog.froxelDimensionZ), 
    //     fog.froxelNear, 
    //     fog.froxelFar, 
    //     fog.froxelInverseViewProjection,
    //     vec2(0.0));

    // vec4 sceenSpaceCenterLast = fog.prevFroxelViewProjection * vec4(worldPosition, 1.0);
    // vec3 ndc = sceenSpaceCenterLast.xyz / sceenSpaceCenterLast.w;

    // float linearDepth = rawDepthToLinearDepth(ndc.z, fog.froxelNear, fog.froxelFar);
    // float depthUv = linearDepthToUv(fog.froxelNear, fog.froxelFar, linearDepth, int(fog.froxelDimensionZ));
    // vec3 historyUv = vec3(ndc.x * 0.5 + 0.5, ndc.y * -0.5 + 0.5, depthUv);

    // if (all(greaterThanEqual(historyUv, vec3(0.0))) && all(lessThanEqual(historyUv, vec3(1.0)))) {
    //     vec4 history = textureLod(prevLightScattering, historyUv, 0.0);

    //     history = max(history, scatteringExtinction);

    //     scatteringExtinction.rgb = mix(history.rgb, scatteringExtinction.rgb, fog.temporalReprojectionPercentage);
    //     scatteringExtinction.a = mix(history.a, scatteringExtinction.a, fog.temporalReprojectionPercentage);
    // }

    imageStore(lightScatteringFiltering, froxelCoord, scatteringExtinction);
}
