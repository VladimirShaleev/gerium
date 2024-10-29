/*
 * This code is based on the Niagara project https://github.com/zeux/niagara
 *
 *    MIT License
 * 
 *    Copyright (c) 2018 Arseny Kapoulkine
 *
 *    https://github.com/zeux/niagara/blob/6e1f3f5f5a21363b328e251377bfdf0093b6b405/src/shaders/meshlet.task.glsl
 */

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

layout(std430, binding = 2, set = GLOBAL_DATA_SET) readonly buffer Instances {
    Instance instances[];
};

layout(std430, binding = 3, set = GLOBAL_DATA_SET) readonly buffer Meshs {
    Mesh meshes[];
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
    uint mgi = gl_LocalInvocationID.x;
    uint commandId = gl_WorkGroupID.x * 64 + gl_WorkGroupID.y;
    MeshTaskCommand command = commands[commandId];

    uint drawId = command.drawId;
    uint taskOffset = command.taskOffset;
    uint taskCount = command.taskCount;

    sharedCount = 0;
    barrier();

    if (mgi < taskCount) {  
        uint mi = mgi + taskOffset;
        uint mvi = mgi + command.visibilityOffset;
        uint meshletVisibilityBit = meshletVisibility[mvi >> 5] & (1u << (mvi & 31));

        bool visible = true;
    #ifndef LATE
        // At the EARLY stage we learn about visibility from the visibility table of meshlets
        visible = meshletVisibilityBit != 0;
    #endif

        Instance instance = instances[drawId];
        mat4 worldView = scene.view * instance.world;
        vec3 center = (worldView * vec4(meshlets[mi].center[0], meshlets[mi].center[1], meshlets[mi].center[2], 1.0)).xyz;
        float radius = instance.scale * float(meshlets[mi].radius);
        float coneCutoff = int(meshlets[mi].coneCutoff) / 127.0;
        vec4 coneAxis = worldView * vec4(
            int(meshlets[mi].coneAxis[0]) / 127.0, 
            int(meshlets[mi].coneAxis[1]) / 127.0, 
            int(meshlets[mi].coneAxis[2]) / 127.0, 0.0);

        visible = visible && !coneCulling(center, radius, coneAxis.xyz, coneCutoff, vec3(0, 0, 0));
        visible = visible && center.z * scene.frustum.y - abs(center.x) * scene.frustum.x > -radius;
        visible = visible && center.z * scene.frustum.w - abs(center.y) * scene.frustum.z > -radius;
        visible = visible && center.z + radius > scene.farNear.y && center.z - radius < scene.farNear.x;

    #ifdef LATE
        if (visible) {
            vec4 aabb;
            if (projectSphere(center, radius, scene.farNear.y, scene.p00p11.x, scene.p00p11.y, aabb)) {
                float width = (aabb.z - aabb.x) * scene.pyramidResolution.x;
                float height = (aabb.w - aabb.y) * scene.pyramidResolution.y;

                float level = floor(log2(max(width, height)));

                float depth = textureLod(depthPyramid, (aabb.xy + aabb.zw) * 0.5, level).x;
                float depthSphere = scene.farNear.y / (center.z - radius);

                visible = depthSphere > depth;
            }
        }

        if (visible) {
            atomicOr(meshletVisibility[mvi >> 5], 1u << (mvi & 31));
        } else {
            atomicAnd(meshletVisibility[mvi >> 5], ~(1u << (mvi & 31)));
        }
    #endif

        // At LATE stage we draw the meshlet only if it was not drawn at EARLY stage.
        if (visible
        #ifdef LATE
            && meshletVisibilityBit == 0
        #endif
            ) {
            uint index = atomicAdd(sharedCount, 1);

            payload.clusterIndices[index] = commandId | (mgi << 24);
        }
    }

    barrier();
    EmitMeshTasksEXT(sharedCount, 1, 1);
}
