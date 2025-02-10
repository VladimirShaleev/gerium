#version 450

#include "common/types.h"

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

void main() {
    MeshInstance instance = instances[gl_InstanceIndex];

    MeshLod mesh = meshes[instance.mesh].lods[0];

    uint index    = indices[mesh.primitiveOffset + gl_VertexIndex];
    uint vertex   = meshes[instance.mesh].vertexOffset + index;
    vec4 position = vec4(vertices[vertex].px, vertices[vertex].py, vertices[vertex].pz, 1.0);
    
    gl_Position = scene.viewProjection * instance.world * position;
}
