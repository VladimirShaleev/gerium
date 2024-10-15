#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

#if defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)
#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require
#endif

struct SceneData {
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
    ivec2 pad0;
};

struct DrawData {
    float lodTarget;
    uint  drawCount;
};

struct MeshTaskCommand {
    uint drawId;
    uint taskOffset;
    uint taskCount;
    uint _pad0;
};

#if (defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)) || defined(__cplusplus)
struct VertexOptimized {
    vec4    position;
    u8vec4  normal;
    f16vec2 texcoord;
    uint    _pad0;
    uint    _pad1;
};

struct MeshletOptimized {
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
struct VertexLegacy {
    vec4 position;
    vec4 normal;
    vec2 texcoord;
};

struct MeshletLegacy {
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

struct ClusterMesh {
    uint           lodCount;
    uint           _pad0; 
    uint           _pad1; 
    uint           _pad2; 
    ClusterMeshLod lods[8];
};

struct ClusterMeshInstance {
    mat4  world;
    mat4  inverseWorld;
    uvec4 textures;
    uint  mesh; 
    uint  _pad0; 
    uint  _pad1; 
    uint  _pad2; 
};

struct MeshTaskPayload {
    mat4 world;
    uint meshletOffset;
    uint _pad0; 
    uint _pad1; 
    uint _pad2; 
};

#endif
