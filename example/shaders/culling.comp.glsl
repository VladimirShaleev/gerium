#version 450

#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require

#include "common/types.h"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = MESH_DATA_SET) uniform DrawDataUBO {
    DrawData drawData[];
};

layout(std430, binding = 1, set = MESH_DATA_SET) readonly buffer ClusterMeshInstances {
    ClusterMeshInstance instances[];
};

void main() {
    uint index = gl_GlobalInvocationID.x;
}
