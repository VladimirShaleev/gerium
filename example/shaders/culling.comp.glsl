#version 450

#include "common/types.h"

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
    uint visibility[];
};

#ifdef LATE
layout(binding = 4, set = GLOBAL_DATA_SET) uniform sampler2D depthPyramid;
#endif

layout(std430, binding = 0, set = MESH_DATA_SET) readonly buffer ClusterMeshInstances {
    ClusterMeshInstance instances[];
};

layout(std430, binding = 1, set = MESH_DATA_SET) readonly buffer ClusterMeshs {
    ClusterMesh meshes[];
};

layout(std430, binding = 2, set = MESH_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

const bool late =
#ifdef LATE
    true;
#else
    false;
#endif

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (index >= drawData.drawCount) {
        return;
    }

    visibility[index] = 1;

    if (!late && visibility[index] == 0) {
        return;
    }

    ClusterMeshInstance instance = instances[index];
    uint meshIndex = instance.mesh;
    ClusterMesh mesh = meshes[meshIndex];

    vec3 center = (scene.view * instance.world * vec4(mesh.centerAndRadius.xyz, 1.0)).xyz;
    float radius = instance.scale * mesh.centerAndRadius.w;

    bool visible = true;
    visible = visible && center.z * scene.frustum.y - abs(center.x) * scene.frustum.x > -radius;
    visible = visible && center.z * scene.frustum.w - abs(center.y) * scene.frustum.z > -radius;
    visible = visible && center.z + radius > scene.farNear.y && center.z - radius < scene.farNear.x;

    if (late && visible) {

    }

    if (visible && (!late || visibility[index] == 0)) {
        uint lodIndex = 0;

        float distance = max(length(center) - radius, 0);
        float threshold = distance * scene.lodTarget / instance.scale;

        for (uint i = 1; i < mesh.lodCount; ++i) {
            if (mesh.lods[i].lodError < threshold) {
                lodIndex = i;
            }
        }

        ClusterMeshLod lod = meshes[meshIndex].lods[lodIndex];

        uint taskGroups = (lod.meshletCount + TASK_GROUP_SIZE - 1) / TASK_GROUP_SIZE;
        uint count = atomicAdd(commandCount, taskGroups);

        for (uint i = 0; i < taskGroups; ++i) {
            commands[count + i].drawId = index;
            commands[count + i].taskOffset = lod.meshletOffset + i * TASK_GROUP_SIZE;
            commands[count + i].taskCount = min(TASK_GROUP_SIZE, lod.meshletCount - i * TASK_GROUP_SIZE);
            commands[count + i].visibility = visibility[index];
            commands[count + i].visibilityOffset = instance.visibilityOffset + i * TASK_GROUP_SIZE;
        }
    }
    
	if (late) {
		visibility[index] = visible ? 1 : 0;
    }
}
