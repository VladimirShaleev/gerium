#version 450

#extension GL_EXT_mesh_shader: require

#include "common/types.h"
#include "common/utils.h"

layout(local_size_x = TASK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std430, binding = 0, set = GLOBAL_DATA_SET) readonly buffer Commands {
    MeshTaskCommand commands[];
};

layout(std430, binding = 1, set = GLOBAL_DATA_SET) buffer MeshletVisibility {
    uint meshletVisibility[];
};

layout(std430, binding = 2, set = GLOBAL_DATA_SET) readonly buffer ClusterMeshInstances {
    ClusterMeshInstance instances[];
};

layout(std430, binding = 3, set = GLOBAL_DATA_SET) readonly buffer ClusterMeshs {
    ClusterMesh meshes[];
};

layout(std430, binding = 4, set = GLOBAL_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

#ifdef LATE
layout(binding = 8, set = GLOBAL_DATA_SET) uniform sampler2D depthPyramid;
#endif

taskPayloadSharedEXT MeshTaskPayload payload;

shared int sharedCount;

void main() {
    uint commandId = gl_WorkGroupID.x * 64 + gl_WorkGroupID.y;
    MeshTaskCommand command = commands[commandId];
    uint drawId = command.drawId;
    uint taskOffset = command.taskOffset;
    uint taskCount = command.taskCount;

    ClusterMeshInstance instance = instances[drawId];
    uint meshIndex = instance.mesh;
    ClusterMesh mesh = meshes[meshIndex];

    uint mgi = gl_LocalInvocationID.x;
    uint mi = mgi + taskOffset;
	uint mvi = mgi + command.visibilityOffset;
    bool valid = mgi < taskCount;

    uint meshletVisibilityBit = meshletVisibility[mvi >> 5] & (1u << (mvi & 31));
    
    sharedCount = 0;
    barrier();

    bool visible = true;
#ifndef LATE
    visible = meshletVisibilityBit != 0;
#endif

#ifndef DEBUG_OCCLUSION
    vec3 center = (scene.view * instance.world * vec4(meshlets[mi].centerAndRadius.xyz, 1.0)).xyz;
    float radius = instance.scale * meshlets[mi].centerAndRadius.w;
    float coneCutoff = int(meshlets[mi].coneAxisAndCutoff.w) / 127.0;
    vec4 coneAxis = scene.view * vec4(rotateQuaternion(vec3(
        int(meshlets[mi].coneAxisAndCutoff.x) / 127.0, 
        int(meshlets[mi].coneAxisAndCutoff.y) / 127.0, 
        int(meshlets[mi].coneAxisAndCutoff.z) / 127.0), instance.orientation), 0);

    visible = visible && !coneCulling(center, radius, coneAxis.xyz, coneCutoff, vec3(0, 0, 0));
    visible = visible && center.z * scene.frustum.y - abs(center.x) * scene.frustum.x > -radius;
    visible = visible && center.z * scene.frustum.w - abs(center.y) * scene.frustum.z > -radius;
    visible = visible && center.z + radius > scene.farNear.y && center.z - radius < scene.farNear.x;
#endif

#ifdef LATE
    if (visible) {
        vec4 aabb;
        if (projectSphere(center, radius, scene.farNear.y, scene.p00p11.x, scene.p00p11.y, aabb)) {
            float width = (aabb.z - aabb.x) * scene.pyramidResolution.x;
            float height = (aabb.w - aabb.y) * scene.pyramidResolution.y;

            float level = max(floor(log2(max(width, height))) - 1, 0);

            float depth = textureLod(depthPyramid, (aabb.xy + aabb.zw) * 0.5, level).x;
            float depthSphere = scene.farNear.y / (center.z - radius);

            visible = depthSphere > depth;
        }
    }

    if (valid) {
        if (visible) {
            atomicOr(meshletVisibility[mvi >> 5], 1u << (mvi & 31));
        } else {
            atomicAnd(meshletVisibility[mvi >> 5], ~(1u << (mvi & 31)));
        }
    }
#endif

    if (valid && visible
    #ifdef LATE
        && meshletVisibilityBit == 0
    #endif
        ) {
        uint index = atomicAdd(sharedCount, 1);

        payload.clusterIndices[index] = commandId | (mgi << 24);
    }

    barrier();
    EmitMeshTasksEXT(sharedCount, 1, 1);
}
