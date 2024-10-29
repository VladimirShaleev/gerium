/*
 * This code is based on the Niagara project https://github.com/zeux/niagara
 * Some changes have been made here to make the first frame render more efficient.
 *
 *    MIT License
 * 
 *    Copyright (c) 2018 Arseny Kapoulkine
 *
 *    https://github.com/zeux/niagara/blob/6e1f3f5f5a21363b328e251377bfdf0093b6b405/src/shaders/drawcull.comp.glsl
 */

#version 450

#include "common/types.h"
#include "common/utils.h"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std140, binding = 0, set = GLOBAL_DATA_SET) uniform DrawDataUBO {
    DrawData drawData;
};

layout(std430, binding = 1, set = GLOBAL_DATA_SET) buffer CommandCount {
    uint commandCount;
};

layout(std430, binding = 2, set = GLOBAL_DATA_SET) writeonly buffer Commands {
    MeshTaskCommand commands[];
};

layout(std430, binding = 3, set = GLOBAL_DATA_SET) buffer Visibility {
    uint8_t visibility[];
};

#ifdef LATE
layout(binding = 4, set = GLOBAL_DATA_SET) uniform sampler2D depthPyramid;
#endif

layout(std430, binding = 0, set = MESH_DATA_SET) readonly buffer Instances {
    Instance instances[];
};

layout(std430, binding = 1, set = MESH_DATA_SET) readonly buffer Meshs {
    Mesh meshes[];
};

layout(std430, binding = 2, set = MESH_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (index >= drawData.drawCount) {
        return;
    }

#ifndef LATE
    if (uint(visibility[index]) == 0) {
        return;
    }
#endif

    Instance instance = instances[index];
    uint meshIndex = instance.mesh;

    vec3 center = (scene.view * instance.world * vec4(meshes[meshIndex].center[0], meshes[meshIndex].center[1], meshes[meshIndex].center[2], 1.0)).xyz;
    float radius = instance.scale * float(meshes[meshIndex].radius);

    // Technically, this check is not necessary in an EARLY pass, but it can culling
    // instances that have gone out of the camera's field of view.
    bool visible = true;
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
#endif

    uint lodCount = uint(meshes[meshIndex].lodCount);
    uint lodIndex = 0;

    // Here we create tasks for rendering if the instance is visible regardless 
    // of whether it is the EARLY pass or LATE pass. The task shader will reject meshlets
    // anyway. This is done so that the meshlet visibility table (meshletVisibility)
    // can be updated when the camera moves.
    if (visible) {
    #ifdef LATE
        // On the LATE pass we calculate the LOD level
        float distance = max(length(center) - radius, 0);
        float threshold = distance * scene.lodTarget / instance.scale;

        for (uint i = 1; i < lodCount; ++i) {
            if (meshes[meshIndex].lods[i].lodError < threshold) {
                lodIndex = i;
            }
        }
    #else
        // On the EARLY pass we select the LOD level from the visibility table.
        // This is different from the Niagara project. This is done so that when
        // rendering the first frame we select all the instances with the least
        // detail (on startup, all bits of the visibility and meshletVisibility
        // tables are filled with ones), and on the LATE pass we calculate LODs.
        // This will allow the first frame to build a depth pyramid more efficiently
        // using low-detail meshes. This will also make the camera movement more
        // adaptive to the selection of LODs.
        lodIndex = min(uint(visibility[index]), lodCount) - 1;
    #endif

        MeshLod lod = meshes[meshIndex].lods[lodIndex];

        uint taskGroups = (lod.meshletCount + TASK_GROUP_SIZE - 1) / TASK_GROUP_SIZE;
        uint count = atomicAdd(commandCount, taskGroups);

        for (uint i = 0; i < taskGroups; ++i) {
            commands[count + i].drawId = index;
            commands[count + i].taskOffset = lod.meshletOffset + i * TASK_GROUP_SIZE;
            commands[count + i].taskCount = min(TASK_GROUP_SIZE, lod.meshletCount - i * TASK_GROUP_SIZE);
            commands[count + i].visibilityOffset = instance.visibilityOffset + i * TASK_GROUP_SIZE;
        }
    }
    
#ifdef LATE
    // In the instance (draw) visibility table we store the instance LOD level plus 1
    visibility[index] = visible ? uint8_t(lodIndex + 1) : uint8_t(0);
#endif
}
