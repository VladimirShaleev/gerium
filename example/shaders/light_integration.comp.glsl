#version 450

#include "common/types.h"
#include "common/utils.h"

layout(local_size_x = FROXEL_DISPATCH_X, local_size_y = FROXEL_DISPATCH_Y, local_size_z = 1) in;

layout(set = 0, binding = 0, std140) uniform VolumetricFogDataUBO {
    VolumetricFogData fogData;
};

layout(set = 1, binding = 0) uniform sampler3D lightScattering;

layout(set = 1, binding = 1, rgba16f) uniform writeonly image3D integratedLightScattering;

void main() {
    ivec3 froxelCoord = ivec3(gl_GlobalInvocationID.xyz);

    vec3 integratedScattering = vec3(0.0, 0.0, 0.0);
    float integratedTransmittance = 1.0;

    float currentZ = 0;

    vec3 rcpFroxelDim = 1.0 / vec3(fogData.froxelDimensionX, fogData.froxelDimensionY, fogData.froxelDimensionZ);

    for (int z = 0; z < fogData.froxelDimensionZ; ++z) {
        froxelCoord.z = z;

        float nextZ = sliceExponentialDepth(fogData.froxelNear, fogData.froxelFar, 0.0, float(z + 1), float(fogData.froxelDimensionZ));
        
        float zStep = abs(nextZ - currentZ);
        currentZ = nextZ;

        const vec4 sampledScatteringExtinction = texture(lightScattering, froxelCoord * rcpFroxelDim);
        const vec3 sampledScattering = sampledScatteringExtinction.xyz;
        const float sampledExtinction = sampledScatteringExtinction.w;
        const float clampedExtinction = max(sampledExtinction, 0.00001);
        
        const float transmittance = exp(-sampledExtinction * zStep);

        const vec3 scattering = (sampledScattering - (sampledScattering * transmittance)) / clampedExtinction;

        integratedScattering += scattering * integratedTransmittance;
        integratedTransmittance *= transmittance;

        vec3 storedScattering = integratedScattering;

        const float opacity = max(1 - integratedTransmittance, 0.00000001);
        storedScattering = integratedScattering / opacity;

        imageStore(integratedLightScattering, froxelCoord.xyz, vec4(storedScattering, integratedTransmittance));
    }
}
