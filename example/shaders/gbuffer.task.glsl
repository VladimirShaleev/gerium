#version 450

#extension GL_EXT_mesh_shader: require

#include "common/types.h"

layout(local_size_x = TASK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std430, binding = 0, set = GLOBAL_DATA_SET) readonly buffer Commands {
    MeshTaskCommand commands[];
};

layout(std430, binding = 1, set = GLOBAL_DATA_SET) readonly buffer ClusterMeshInstances {
    ClusterMeshInstance instances[];
};

layout(std430, binding = 2, set = GLOBAL_DATA_SET) readonly buffer ClusterMeshs {
    ClusterMesh meshes[];
};

layout(std430, binding = 3, set = GLOBAL_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

taskPayloadSharedEXT MeshTaskPayload payload;

void main() {
    MeshTaskCommand command = commands[gl_WorkGroupID.x * 64 + gl_WorkGroupID.y];
    uint drawId = command.drawId;
    uint taskOffset = command.taskOffset;
    uint taskCount = command.taskCount;

    // uint meshIndex = instances[drawId].mesh;
    // ClusterMesh mesh = meshes[meshIndex];

    uint mgi = gl_LocalInvocationID.x;
    uint mi = gl_LocalInvocationID.x + taskOffset;
    
    payload.drawId = drawId;
    payload.meshletIndices[gl_LocalInvocationID.x] = mi;

    EmitMeshTasksEXT(taskCount, 1, 1);
}
