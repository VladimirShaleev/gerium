#version 450

#extension GL_ARB_shader_draw_parameters : require

#include "common/types.h"

layout(location = 0) flat out uint material;

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

layout(std430, binding = 1, set = INSTANCES_DATA_SET) readonly buffer CommandsSSBO {
    IndirectDraw commands[];
};

void main() {
    MeshInstance instance = instances[gl_InstanceIndex];

    uint commandIndex = instance.technique * MAX_INSTANCES_PER_TECHNIQUE + gl_DrawIDARB;

    uint lodIndex = commands[commandIndex].lodIndex;

    MeshLod mesh = meshes[instance.mesh].lods[lodIndex];

    uint index    = indices[mesh.primitiveOffset + gl_VertexIndex];
    uint vertex   = meshes[instance.mesh].vertexOffset + index;
    vec4 position = vec4(vertices[vertex].px, vertices[vertex].py, vertices[vertex].pz, 1.0);
    
    gl_Position = scene.viewProjection * instance.world * position;
}
