#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

struct SceneData {
    GLM_NAMESPACE mat4  view;
    GLM_NAMESPACE mat4  viewProjection;
    GLM_NAMESPACE mat4  prevViewProjection;
    GLM_NAMESPACE mat4  invViewProjection;
    GLM_NAMESPACE vec4  viewPosition;
    GLM_NAMESPACE vec4  eye;
    GLM_NAMESPACE vec2  planes;
    GLM_NAMESPACE vec2  jitter;
    GLM_NAMESPACE vec2  prevJitter;
    GLM_NAMESPACE vec2  invResolution;
    GLM_NAMESPACE ivec2 resolution;
    GLM_NAMESPACE ivec2 pad0;
};

#if (defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)) || defined(__cplusplus)
struct VertexOptimized {
    GLM_NAMESPACE vec4    position;
    GLM_NAMESPACE u8vec4  normal;
    GLM_NAMESPACE f16vec2 texcoord;
    GLM_NAMESPACE uint    _pad0;
    GLM_NAMESPACE uint    _pad1;
};

struct MeshletOptimized {
    GLM_NAMESPACE vec4   centerAndRadius;
	GLM_NAMESPACE i8vec4 coneAxisAndCutoff;
    GLM_NAMESPACE uint   vertexOffset;
    GLM_NAMESPACE uint   primitiveOffset;
    uint16_t             vertexCount;
    uint16_t             primitiveCount;
};

#ifndef __cplusplus
#define Vertex VertexOptimized
#define Meshlet MeshletOptimized
#endif
#endif

#if !defined(SHADER_8BIT_STORAGE_SUPPORTED) || !defined(SHADER_16BIT_STORAGE_SUPPORTED) || defined(__cplusplus)
// TODO: add structs for legacy pipeline
struct VertexLegacy {
    GLM_NAMESPACE vec4 position;
    GLM_NAMESPACE vec4 normal;
    GLM_NAMESPACE vec2 texcoord;
};

struct MeshletLegacy {
    GLM_NAMESPACE vec4 centerAndRadius;
	GLM_NAMESPACE vec4 coneAxisAndCutoff;
    GLM_NAMESPACE uint vertexOffset;
    GLM_NAMESPACE uint primitiveOffset;
    uint16_t           vertexCount;
    uint16_t           primitiveCount;
};
#ifndef __cplusplus
#define Vertex VertexLegacy
#define Meshlet MeshletLegacy
#endif
#endif

#endif
