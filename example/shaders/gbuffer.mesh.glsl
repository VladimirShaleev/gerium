#version 450

#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_mesh_shader: require

#include "types.h"

struct Meshlet {
    vec3 center;
    float radius;
    int8_t cone_axis[3];
    int8_t cone_cutoff;

    uint dataOffset;
    uint8_t vertexCount;
    uint8_t triangleCount;
};

layout(local_size_x = MESH_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
layout(triangles, max_vertices = MESH_MAX_VERTICES, max_primitives = MESH_MAX_PRIMITIVES) out;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

layout(std140, binding = 0, set = MESH_DATA_SET) readonly buffer Vertices {
    Vertex vertices[];
};

layout(std140, binding = 1, set = MESH_DATA_SET) readonly buffer MeshletData {
    uint meshletData[];
};

layout(std140, binding = 2, set = MESH_DATA_SET) readonly buffer Meshlets {
    Meshlet meshlets[];
};

layout(location = 0) out vec4 color[];

uint hash(uint a)
{
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
}

void main() {
    uint mi = gl_WorkGroupID.x;

    uint mhash = hash(mi);
    vec3 mcolor = vec3(float(mhash & 255), float((mhash >> 8) & 255), float((mhash >> 16) & 255)) / 255.0;
    
    // uint vertexCount = uint(meshlets[mi].vertexCount);
    // uint triangleCount = uint(meshlets[mi].triangleCount);
    // 
    // SetMeshOutputsEXT(vertexCount, triangleCount);
    // 
    // uint dataOffset = meshlets[mi].dataOffset;
    // uint vertexOffset = dataOffset;
    // uint indexOffset = dataOffset + vertexCount;
    // 
    // uint mhash = hash(mi);
    // vec3 mcolor = vec3(float(mhash & 255), float((mhash >> 8) & 255), float((mhash >> 16) & 255)) / 255.0;
    // 
    // for (uint i = 0; i < vertexCount; i += MESH_GROUP_SIZE) {
    //     uint vi = meshletData[vertexOffset + i];
    // 
    //     vec4 clip = scene.viewProjection * vertices[vi].position;
    // 
    //     gl_MeshVerticesEXT[i].gl_Position = clip;
    //     color[i] = vec4(mcolor, 1.0);
    // }
    // 
    // for (uint i = 0; i < triangleCount; i += MESH_GROUP_SIZE) {
    //     uint offset = indexOffset * 4 + i * 3;
    //     uint a = uint(meshletData[offset]), b = uint(meshletData[offset + 1]), c = uint(meshletData[offset + 2]);
    // 
    //     gl_PrimitiveTriangleIndicesEXT[i] = uvec3(a, b, c);
    // }

    if (mi == 0) {
        SetMeshOutputsEXT(3, 1);

        vec4 positions[3];
        positions[0] = vec4(0.0, 0.0, 0.5, 1.0);
        positions[1] = vec4(1.0, 0.0, 0.5, 1.0);
        positions[2] = vec4(1.0, 1.0, 0.5, 1.0);

        for (uint i = 0; i < 3; ++i) {
            gl_MeshVerticesEXT[i].gl_Position = scene.viewProjection * positions[i];
            color[i] = vec4(mcolor, 1.0);
        }

        for (uint i = 0; i < 1; ++i) {
            gl_PrimitiveTriangleIndicesEXT[i] = uvec3(0, 1, 2);
        }
    }
}
