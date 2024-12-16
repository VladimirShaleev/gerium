#version 450

#include "common/types.h"
#include "common/utils.h"

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(set = 1, binding = 0, std140) uniform VolumetricFogDataUBO {
    VolumetricFogData fogData;
};

layout(set = 2, binding = 0) uniform sampler3D froxelData;

layout(set = 2, binding = 1) uniform sampler2DArray csm;

layout(set = 2, binding = 2, rgba16f) uniform writeonly image3D lightScattering;

layout(std140, set = 2, binding = 3) uniform LightCountUBO {
    uint lightCount;
};

layout(std430, set = 2, binding = 4) readonly buffer LightSSBO {
    Light lights[];
};

layout (std140, set = 3, binding = 0) uniform LightSpaceMatricesUBO {
    mat4 lightSpaceMatrices[CSM_MAX_CASCADES];
};

layout(std140, set = 3, binding = 1) uniform CascadeDistancesUBO {
    float cascadeDistances[CSM_MAX_CASCADES];
};

layout(local_size_x = FROXEL_DISPATCH_X, local_size_y = FROXEL_DISPATCH_Y, local_size_z = FROXEL_DISPATCH_Z) in;

float shadow(vec3 worldPosition, float depth) {
    int layer = 0;
    for (int i = 0; i < CSM_MAX_CASCADES - 1; ++i) {
        if (depth < cascadeDistances[i]) {
            break;
        }
        layer = i + 1;
    }

    vec4 fragPosLight = lightSpaceMatrices[layer] * vec4(worldPosition, 1.0);
    vec4 projCoords = fragPosLight / fragPosLight.w;
    projCoords.x = projCoords.x * 0.5 + 0.5;
    projCoords.y = 1.0 - (projCoords.y * 0.5 + 0.5);

    float bias = 0.005;
    bias *= 1.0 / (cascadeDistances[layer] * 0.5);

    if (projCoords.z > 0.0 && projCoords.z < 1.0) {
        float pcfDepth = texture(csm, vec3(projCoords.xy, layer)).r; 
        float shadow = projCoords.w > 0.0 && pcfDepth < projCoords.z - bias ? 0.0 : 1.0;   
        return shadow;
    }
    return 1.0;
}

// Equations from http://patapom.com/topics/Revision2013/Revision%202013%20-%20Real-time%20Volumetric%20Rendering%20Course%20Notes.pdf
float henyeyGreenstein(float g, float costh) {
    const float numerator = 1.0 - g * g;
    const float denominator = 4.0 * PI * pow(1.0 + g * g - 2.0 * g * costh, 3.0 / 2.0);
    return numerator / denominator;
}

float phaseFunction(vec3 V, vec3 L, float g) {
    float cosTheta = dot(V, L);
    return henyeyGreenstein(g, cosTheta);
}

void main() {
    ivec3 froxelCoord = ivec3(gl_GlobalInvocationID.xyz);
    
    vec3 worldPosition = worldPositionFromFroxel(
        froxelCoord, 
        uvec3(fogData.froxelDimensionX, fogData.froxelDimensionY, fogData.froxelDimensionZ), 
        fogData.froxelNear, 
        fogData.froxelFar, 
        fogData.froxelInverseViewProjection,
        fogData.haltonXY);

    vec3 rcpFroxelDim = 1.0 / vec3(fogData.froxelDimensionX, fogData.froxelDimensionY, fogData.froxelDimensionZ);
    vec3 fogDataUvw = froxelCoord * rcpFroxelDim;
    vec4 scatteringExtinction = texture(froxelData, fogDataUvw);

    float extinction = scatteringExtinction.a;
    vec3 lighting = vec3(0);

    if (extinction >= 0.01f) {
        vec3 V = normalize(scene.viewPosition.xyz - worldPosition);

        for (int i = 0; i < lightCount; ++i) {
            Light light = lights[i];

            float depth = (scene.view * vec4(worldPosition, 1.0)).z;
            float shadowFactor = shadow(worldPosition, depth);

            vec3 pointToLight = light.directionRange.xyz;

            lighting += light.colorIntensity.rgb * light.colorIntensity.a * phaseFunction(V, -pointToLight, fogData.phaseAnisotropy) * shadowFactor;
        }
    }
    
    vec3 scattering = scatteringExtinction.rgb * lighting;

    imageStore(lightScattering, ivec3(froxelCoord.xyz), vec4(scattering, extinction));
}
