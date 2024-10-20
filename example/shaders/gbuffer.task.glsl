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

layout(std430, binding = 1, set = GLOBAL_DATA_SET) buffer Visibility {
    uint visibility[];
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

taskPayloadSharedEXT MeshTaskPayload payload;

shared int sharedCount;

const bool late =
#ifdef LATE
    true;
#else
    false;
#endif

void main() {
    uint commandId = gl_WorkGroupID.x * 64 + gl_WorkGroupID.y;
    MeshTaskCommand command = commands[commandId];
    uint drawId = command.drawId;
    uint taskOffset = command.taskOffset;
    uint taskCount = command.taskCount;
    uint lateDrawVisibility = command.visibility;

    ClusterMeshInstance instance = instances[drawId];
    uint meshIndex = instance.mesh;
    ClusterMesh mesh = meshes[meshIndex];

    uint mgi = gl_LocalInvocationID.x;
    uint mi = gl_LocalInvocationID.x + taskOffset;
    uint mvi = mgi + command.visibilityOffset;

    sharedCount = 0;
    barrier();

    vec3 center = (scene.view * instance.world * vec4(mesh.centerAndRadius.xyz, 1.0)).xyz;
    float radius = instance.scale * mesh.centerAndRadius.w;
    float coneCutoff = int(meshlets[mi].coneAxisAndCutoff.w) / 127.0;
    vec3 coneAxis = rotateQuaternion(vec3(
        int(meshlets[mi].coneAxisAndCutoff.x) / 127.0, 
        int(meshlets[mi].coneAxisAndCutoff.y) / 127.0, 
        int(meshlets[mi].coneAxisAndCutoff.z) / 127.0), instance.orientation);

    bool valid = mgi < taskCount;
    bool visible = valid;
    bool skip = false;

    uint meshletVisibilityBit = visibility[mvi >> 5] & (1u << (mvi & 31));

    if (!late && meshletVisibilityBit == 0) {
        visible = false;
    }

    if (late && lateDrawVisibility == 1 && meshletVisibilityBit != 0) {
        skip = true;
    }

    visible = visible && !coneCulling(center, radius, coneAxis, coneCutoff, vec3(0, 0, 0));
    visible = visible && center.z * scene.frustum.y - abs(center.x) * scene.frustum.x > -radius;
    visible = visible && center.z * scene.frustum.w - abs(center.y) * scene.frustum.z > -radius;
    visible = visible && center.z + radius > scene.farNear.y && center.z - radius < scene.farNear.x;

    if (late && visible) {

    }

    if (late && valid) {

    }

    if (visible && !skip) {
		uint index = atomicAdd(sharedCount, 1);
    }

    payload.drawId = drawId;
    payload.meshletIndices[gl_LocalInvocationID.x] = mi;

    EmitMeshTasksEXT(taskCount, 1, 1);
}
