/*
 * This code is based on the Niagara project https://github.com/zeux/niagara
 *
 *    MIT License
 * 
 *    Copyright (c) 2018 Arseny Kapoulkine
 *
 *    https://github.com/zeux/niagara/blob/6e1f3f5f5a21363b328e251377bfdf0093b6b405/src/shaders/meshlet.mesh.glsl
 */

#version 450

#extension GL_EXT_mesh_shader: require

#include "common/types.h"
#include "common/utils.h"

layout(local_size_x = MESH_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
layout(triangles) out;
layout(max_vertices = MESH_MAX_VERTICES, max_primitives = MESH_MAX_PRIMITIVES) out;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std430, binding = 0, set = GLOBAL_DATA_SET) readonly buffer Commands {
    MeshTaskCommand commands[];
};

layout(std430, binding = 0, set = CLUSTER_DATA_SET) readonly buffer Vertices {
    Vertex vertices[];
};

layout(std430, binding = 1, set = CLUSTER_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

layout(std430, binding = 2, set = CLUSTER_DATA_SET) readonly buffer MeshletVertices {
    uint meshletVertices[];
};

layout(std430, binding = 3, set = CLUSTER_DATA_SET) readonly buffer MeshletPrimitives {
    uint8_t meshletPrimitives[];
};

layout(std430, binding = 5, set = CLUSTER_DATA_SET) readonly buffer Instances {
    Instance instances[];
};

taskPayloadSharedEXT MeshTaskPayload payload;

layout(location = 0) out vec4 position[];
layout(location = 1) out vec4 prevPosition[];
layout(location = 2) out vec4 meshletColor[];
layout(location = 3) out vec2 texcoord[];
layout(location = 4) out vec3 normal[];
layout(location = 5) out vec3 tangent[];
layout(location = 6) out vec3 bitangent[];
layout(location = 7) flat out uint instanceId[];

void main() {
    uint ti = gl_LocalInvocationID.x;
    uint ci = payload.clusterIndices[gl_WorkGroupID.x];

    MeshTaskCommand command = commands[ci & 0xffffff];
    uint mi = command.taskOffset + (ci >> 24);

    uint mhash = hash(mi);
    vec3 mcolor = vec3(float(mhash & 255), float((mhash >> 8) & 255), float((mhash >> 16) & 255)) / 255.0;
    
    uint vertexOffset    = uint(meshlets[mi].vertexOffset);
    uint vertexCount     = uint(meshlets[mi].vertexCount);
    uint primitiveOffset = uint(meshlets[mi].primitiveOffset);
    uint primitiveCount  = uint(meshlets[mi].primitiveCount);
    
    SetMeshOutputsEXT(vertexCount, primitiveCount);

    const uint vertexBatches    = (MESH_MAX_VERTICES + MESH_GROUP_SIZE - 1) / MESH_GROUP_SIZE;
    const uint primitiveBatches = (MESH_MAX_PRIMITIVES + MESH_GROUP_SIZE - 1) / MESH_GROUP_SIZE;
    
    for (uint batch = 0; batch < vertexBatches; batch++) {
        uint offset = min(ti + batch * MESH_GROUP_SIZE, vertexCount - 1);
        uint index  = meshletVertices[vertexOffset + offset];

        vec4 vPosition = vec4(float(vertices[index].px), float(vertices[index].py), float(vertices[index].pz), 1.0);
        vec2 vTexcoord = vec2(float(vertices[index].tu), float(vertices[index].tv));
        vec3 vNormal   = vec3(int(vertices[index].nx), int(vertices[index].ny), int(vertices[index].nz)) / 127.0 - 1.0;
        vec3 vTangent  = vec3(int(vertices[index].tx), int(vertices[index].ty), int(vertices[index].tz)) / 127.0 - 1.0;

        vec3 worldNormal  = normalize(mat3(instances[command.drawId].normalMatrix) * vNormal);
        vec3 worldTangent = normalize(mat3(instances[command.drawId].normalMatrix) * vTangent);

        vec4 positionResult = scene.viewProjection * instances[command.drawId].world * vPosition;
        vec4 prevPositionResult = scene.prevViewProjection * instances[command.drawId].world * vPosition;

        gl_MeshVerticesEXT[offset].gl_Position = positionResult;
        position[offset]     = positionResult;
        prevPosition[offset] = prevPositionResult;
        meshletColor[offset] = vec4(mcolor, 1.0);
        texcoord[offset]     = vTexcoord;
        normal[offset]       = worldNormal;
        tangent[offset]      = worldTangent;
        bitangent[offset]    = cross(worldNormal, worldTangent) * int(vertices[index].ts);
        instanceId[offset]   = command.drawId;
    }

    for (uint batch = 0; batch < primitiveBatches; batch++) {
        uint offset = min(ti + batch * MESH_GROUP_SIZE, primitiveCount - 1);
        uint index0 = uint(meshletPrimitives[primitiveOffset + offset * 3 + 0]);
        uint index1 = uint(meshletPrimitives[primitiveOffset + offset * 3 + 1]);
        uint index2 = uint(meshletPrimitives[primitiveOffset + offset * 3 + 2]);
        gl_PrimitiveTriangleIndicesEXT[offset] = uvec3(index0, index1, index2);
    }
}
