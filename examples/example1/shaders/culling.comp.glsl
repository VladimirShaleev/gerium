#version 450

#include "common/types.h"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneUBO {
    SceneData scene;
};

layout(std430, binding = 0, set = CLUSTER_DATA_SET) readonly buffer VerticesSSBO {
    Vertex vertices[];
};

layout(std430, binding = 1, set = CLUSTER_DATA_SET) readonly buffer IndicesSSBO {
    uint indices[];
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
    
    uint count = atomicAdd(commandCount, 1);
    
    MeshInstance instance = instances[index];
    MeshLod mesh = meshes[instance.mesh].lods[0];
    
    commands[count].vertexCount   = mesh.primitiveCount;
    commands[count].instanceCount = 1;
    commands[count].firstVertex   = 0;
    commands[count].firstInstance = index;
}
