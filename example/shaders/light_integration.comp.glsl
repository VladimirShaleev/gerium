#version 450

#include "common/types.h"
#include "common/utils.h"

layout(set = 0, binding = 0, std140) uniform VolumetricFogDataUBO {
    VolumetricFogData fog;
};

layout(set = 1, binding = 0) uniform sampler3D froxelData;

layout(set = 1, binding = 1, rgba16f) uniform writeonly image3D integratedLight;

layout(local_size_x = FROXEL_LOCAL_X, local_size_y = FROXEL_LOCAL_Y, local_size_z = FROXEL_LOCAL_Z) in;

float sliceDistance(int z) {
    float n = fog.biasNearFarPow.y;
    float f = fog.biasNearFarPow.z;
    return n * pow(f / n, (float(z) + 0.5) / float(FROXEL_GRID_SIZE_Z));
}

float sliceThickness(int z) {
    return abs(sliceDistance(z + 1) - sliceDistance(z));
}

// https://github.com/Unity-Technologies/VolumetricLighting/blob/master/Assets/VolumetricFog/Shaders/Scatter.compute
vec4 accumulate(int z, vec3 accumScattering, float accumTransmittance, vec3 sliceScattering, float sliceDensity) {
    const float thickness = sliceThickness(z);
    const float sliceTransmittance = exp(-sliceDensity * thickness * 0.01);

    vec3 sliceScatteringIntegral = sliceScattering * (1.0 - sliceTransmittance) / sliceDensity;

    accumScattering += sliceScatteringIntegral * accumTransmittance;
    accumTransmittance *= sliceTransmittance;

    return vec4(accumScattering, accumTransmittance);
}

void main() {
    vec4 accumScatteringTransmittance = vec4(0.0, 0.0, 0.0, 1.0);

    for (int z = 0; z < FROXEL_GRID_SIZE_Z; z++) {
        ivec3 coord = ivec3(gl_GlobalInvocationID.xy, z);

        vec4 sliceScatteringDensity = texelFetch(froxelData, coord, 0);

        accumScatteringTransmittance = accumulate(z,
            accumScatteringTransmittance.rgb, 
            accumScatteringTransmittance.a,
            sliceScatteringDensity.rgb,
            sliceScatteringDensity.a);

        imageStore(integratedLight, coord, accumScatteringTransmittance);
    }
}
