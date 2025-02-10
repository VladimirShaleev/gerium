#version 450

#include "common/types.h"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneUBO {
    SceneData scene;
};

layout(std430, binding = 2, set = CLUSTER_DATA_SET) readonly buffer MeshesSSBO {
    Mesh meshes[];
};

layout(std430, binding = 0, set = INSTANCES_DATA_SET) readonly buffer InstancesSSBO {
    MeshInstance instances[];
};

layout(std430, binding = 1, set = INSTANCES_DATA_SET) buffer CommandCountSSBO {
    uint commandCount;
};

layout(std430, binding = 2, set = INSTANCES_DATA_SET) writeonly buffer CommandsSSBO {
    IndirectDraw commands[];
};

layout(std430, binding = 3, set = INSTANCES_DATA_SET) readonly buffer InstanceCountSSBO {
    uint instanceCount;
};

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (index >= instanceCount) {
        return;
    }
    
    MeshInstance instance = instances[index];
    uint meshIndex = instance.mesh;
    
    vec3 center = (scene.view * instance.world * vec4(meshes[meshIndex].center[0], meshes[meshIndex].center[1], meshes[meshIndex].center[2], 1.0)).xyz;
    float radius = instance.scale * float(meshes[meshIndex].radius);

    bool visible = true;
    visible = visible && center.z * scene.frustum.y - abs(center.x) * scene.frustum.x > -radius;
    visible = visible && center.z * scene.frustum.w - abs(center.y) * scene.frustum.z > -radius;
    visible = visible && center.z + radius > scene.farNear.y && center.z - radius < scene.farNear.x;

    if (visible) {
        uint lodCount = uint(meshes[meshIndex].lodCount);
        uint lodIndex = 0;

        float distance = max(length(center) - radius, 0);
        float threshold = distance * scene.lodTarget / instance.scale;

        for (uint i = 1; i < lodCount; ++i) {
            if (meshes[meshIndex].lods[i].lodError < threshold) {
                lodIndex = i;
            }
        }

        uint count = atomicAdd(commandCount, 1);
        
        commands[count].vertexCount   = meshes[meshIndex].lods[lodIndex].primitiveCount;
        commands[count].instanceCount = 1;
        commands[count].firstVertex   = 0;
        commands[count].firstInstance = index;
        commands[count].lodIndex      = lodIndex;
    }
}
