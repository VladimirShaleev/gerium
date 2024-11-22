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

layout(std430, binding = 2, set = GLOBAL_DATA_SET) writeonly buffer IndexedIndirectDrawSSBO {
    IndexedIndirectDraw draws[];
};

layout(std430, binding = 5, set = CLUSTER_DATA_SET) readonly buffer InstanceSSBO {
    Instance instances[];
};

layout(std430, binding = 6, set = CLUSTER_DATA_SET) readonly buffer SimpleMeshSSBO {
    SimpleMesh simpleMeshes[];
};

void main() {
    uint index = gl_GlobalInvocationID.x;

    if (index >= drawData.drawCount) {
        return;
    }

    uint count = atomicAdd(commandCount, 1);

    Instance instance = instances[index];
    SimpleMesh simpleMesh = simpleMeshes[instance.mesh];

    draws[count].indexCount    = simpleMesh.primitiveCount * 3;
    draws[count].instanceCount = 1;
    draws[count].firstIndex    = simpleMesh.primitiveOffset;
    draws[count].vertexOffset  = int(simpleMesh.vertexOffset);
    draws[count].firstInstance = index;
}
