#version 450

#include "common/types.h"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform DrawDataUBO {
    DrawData drawData;
};

layout(std430, binding = 1, set = SCENE_DATA_SET) buffer CommandCount {
    uint commandCount;
};

layout(std430, binding = 2, set = SCENE_DATA_SET) writeonly buffer Commands {
    MeshTaskCommand commands[];
};

layout(std430, binding = 3, set = SCENE_DATA_SET) buffer Visibility {
    uint8_t visibility[];
};

// layout(binding = 4, set = SCENE_DATA_SET) uniform sampler2D depthPyramid;

layout(std430, binding = 0, set = MESH_DATA_SET) readonly buffer ClusterMeshInstances {
    ClusterMeshInstance instances[];
};

layout(std430, binding = 1, set = MESH_DATA_SET) readonly buffer ClusterMeshs {
    ClusterMesh meshes[];
};

layout(std430, binding = 2, set = MESH_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (index >= drawData.drawCount) {
        return;
    }

    uint meshIndex = instances[index].mesh;

    ClusterMeshLod lod = meshes[meshIndex].lods[0];

    uint taskGroups = (lod.meshletCount + TASK_GROUP_SIZE - 1) / TASK_GROUP_SIZE;
    uint count = atomicAdd(commandCount, taskGroups);

    for (uint i = 0; i < taskGroups; ++i) {
        commands[count + i].drawId = index;
        commands[count + i].taskOffset = lod.meshletOffset + i * TASK_GROUP_SIZE;
        commands[count + i].taskCount = min(TASK_GROUP_SIZE, lod.meshletCount - i * TASK_GROUP_SIZE);
    }
}
