#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

#if defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)
#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require
#endif

struct SHADER_ALIGN SceneData {
    mat4  view;
    mat4  viewProjection;
    mat4  prevViewProjection;
    mat4  invViewProjection;
    vec4  viewPosition;
    vec4  eye;
    vec2  planes;
    vec2  jitter;
    vec2  prevJitter;
    vec2  invResolution;
    ivec2 resolution;
};

struct SHADER_ALIGN DrawData {
    float lodTarget;
    uint  drawCount;
};

struct SHADER_ALIGN MeshTaskCommand {
    uint drawId;
    uint taskOffset;
    uint taskCount;
};

#if (defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)) || defined(__cplusplus)
struct SHADER_ALIGN VertexOptimized {
    vec4    position;
    u8vec4  normal;
    f16vec2 texcoord;
};

struct SHADER_ALIGN MeshletOptimized {
    vec4     centerAndRadius;
    i8vec4   coneAxisAndCutoff;
    uint     vertexOffset;
    uint     primitiveOffset;
    uint16_t vertexCount;
    uint16_t primitiveCount;
};

#ifndef __cplusplus
#define Vertex VertexOptimized
#define Meshlet MeshletOptimized
#endif
#endif

#if !defined(SHADER_8BIT_STORAGE_SUPPORTED) || !defined(SHADER_16BIT_STORAGE_SUPPORTED) || defined(__cplusplus)
// TODO: add structs for legacy pipeline
struct SHADER_ALIGN VertexLegacy {
    vec4 position;
    vec4 normal;
    vec2 texcoord;
};

struct SHADER_ALIGN MeshletLegacy {
    vec4     centerAndRadius;
    vec4     coneAxisAndCutoff;
    uint     vertexOffset;
    uint     primitiveOffset;
    uint16_t vertexCount;
    uint16_t primitiveCount;
};

#ifndef __cplusplus
#define Vertex VertexLegacy
#define Meshlet MeshletLegacy
#endif
#endif

struct ClusterMeshLod {
    uint meshletOffset;
    uint meshletCount;
};

struct SHADER_ALIGN ClusterMesh {
    uint           lodCount;
    ClusterMeshLod lods[8];
};

struct SHADER_ALIGN ClusterMeshInstance {
    mat4  world;
    mat4  inverseWorld;
    uvec4 textures;
    uint  mesh; 
};

struct MeshTaskPayload {
    uint drawId;
    uint meshletIndices[TASK_GROUP_SIZE];
};

#endif
