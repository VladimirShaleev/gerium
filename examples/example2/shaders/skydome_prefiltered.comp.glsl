#version 450

#include "common/types.h"
#include "common/utils.h"

layout(local_size_x = SKY_GROUP_SIZE, local_size_y = SKY_GROUP_SIZE, local_size_z = 1) in;

layout(set = 0, binding = 0, std140) uniform SkyPrefilteredDataUBO {
    SkyPrefilteredData skyPrefiltered;
};

layout(set = 0, binding = 1) uniform samplerCube envCube;

layout(set = 0, binding = 2, rgba16f) uniform writeonly imageCube prefilteredCube;

void main() {
    vec2 uv = uvFromCoords(gl_GlobalInvocationID.xy, vec2(skyPrefiltered.invSize.x));

    vec3 N = getDirection(gl_GlobalInvocationID.z, 2.0 * uv.x - 1.0, 1.0 - 2.0 * uv.y);
    vec3 R = N;
    vec3 V = R;

    vec3  prefilteredColor = vec3(0.0);
    float totalWeight      = 0.0;

    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    mat3 tangentToWorld = mat3(tangent, bitangent, N);

    for (uint i = 0; i < 32; ++i) {
        vec3 H = tangentToWorld * skyPrefiltered.sampleDirections[i].xyz;
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0) {
            prefilteredColor += textureLod(envCube, L, 0).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;

    ivec3 coord = ivec3(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, gl_GlobalInvocationID.z);
    imageStore(prefilteredCube, coord, vec4(prefilteredColor, 1.0));
}
