#version 450

#include "common/types.h"
#include "common/utils.h"

layout(set = 0, binding = 0, std140) uniform VolumetricFogDataUBO {
    VolumetricFogData fog;
};

layout(set = 1, binding = 0) uniform sampler3D lightScattering;

layout(set = 1, binding = 1, rgba16f) uniform writeonly image3D lightFiltering;

layout(local_size_x = FROXEL_DISPATCH_X, local_size_y = FROXEL_DISPATCH_Y, local_size_z = FROXEL_DISPATCH_Z) in;

float gaussian(float radius, float sigma) {
    const float v = radius / sigma;
    return exp(-(v * v));
}

void main() {
    ivec3 froxelCoord = ivec3(gl_GlobalInvocationID.xyz);
    vec3 rcpFroxelDim = 1.0 / vec3(fog.froxelDimensionX, fog.froxelDimensionY, fog.froxelDimensionZ);

    vec4 scatteringExtinction = texture(lightScattering, froxelCoord * rcpFroxelDim);

    // float accumulatedWeight = 0;
    // vec4 accumulatedScatteringExtinction = vec4(0);

    // const float sigmaFilter = 4.0;
    // const int radius = 2;

    // for (int i = -radius; i <= radius; ++i ) {
    //     for (int j = -radius; j <= radius; ++j ) {
    //         ivec3 coord = froxelCoord + ivec3(i, j, 0);
    //         if (all(greaterThanEqual(coord, ivec3(0))) && all(lessThanEqual(coord, ivec3(fog.froxelDimensionX, fog.froxelDimensionY, fog.froxelDimensionZ)))) {
    //             const float weight = gaussian(length(ivec2(i, j)), sigmaFilter);
    //             const vec4 sampledValue = texture(lightScattering, coord * rcpFroxelDim);
    //             accumulatedScatteringExtinction.rgba += sampledValue.rgba * weight;
    //             accumulatedWeight += weight;
    //         }
    //     }
    // }

    // scatteringExtinction = accumulatedScatteringExtinction / accumulatedWeight;

    imageStore(lightFiltering, froxelCoord, scatteringExtinction);
}
