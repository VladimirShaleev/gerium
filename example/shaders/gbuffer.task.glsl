#version 450

#extension GL_EXT_mesh_shader: require

#include "common/types.h"

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std430, binding = 0, set = MESH_DATA_SET) readonly buffer ClusterMeshInstances {
    ClusterMeshInstance instances[];
};

layout(std430, binding = 1, set = MESH_DATA_SET) readonly buffer ClusterMeshs {
    ClusterMesh meshes[];
};

layout(std430, binding = 3, set = MESH_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

taskPayloadSharedEXT MeshTaskPayload payload;

void main() {
    uint instanceIndex = gl_WorkGroupID.x;

    uint meshIndex = instances[instanceIndex].mesh;

    if (meshIndex != 0xFFFFFFFF) {
        vec3 center = meshlets[meshes[meshIndex].lods[0].meshletOffset].centerAndRadius.xyz;
        vec4 centerProj = scene.view * instances[instanceIndex].world * vec4(center, 1.0);

        float distance = max(length(centerProj), 0);
        uint lodIndex = min(uint(distance / 5.0), 7);

        uint offset = meshes[meshIndex].lods[lodIndex].meshletOffset;
        uint count  = meshes[meshIndex].lods[lodIndex].meshletCount;

        payload.world = instances[instanceIndex].world;
        payload.meshletOffset = offset;

        EmitMeshTasksEXT(count, 1, 1);
    }
}
