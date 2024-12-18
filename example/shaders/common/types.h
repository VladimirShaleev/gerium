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
    vec4  frustum;
    vec2  p00p11;
    vec2  farNear;
    vec2  jitter;
    vec2  prevJitter;
    vec2  invResolution;
    ivec2 resolution;
    ivec2 pyramidResolution;
    float lodTarget;
    uint  settingsOutput;
};

struct Light {
    vec4 directionRange;
    vec4 colorIntensity;
    uint type;
};

struct SHADER_ALIGN SkyData {
    vec4  sunDirection;
    float rayleigh;
    float turbidity;
    float mieCoefficient;
    float luminance;
    float mieDirectionalG;
};

struct SHADER_ALIGN SkyPrefilteredData {
    vec4 invSize;
    vec4 sampleDirections[32];
};

struct SHADER_ALIGN VolumetricFogData {
    mat4  view;
    mat4  projection;
    mat4  viewProj;
    mat4  invViewProj;
    mat4  prevViewProj;
    vec4  lightDirection;
    vec4  lightColor;
    vec4  cameraPosition;
    vec4  biasNearFarPow;
    vec4  anisoDensityScatteringAbsorption;
    ivec4 widthHeight;
    float time;
};

struct Angular {
    float NdotL;
    float NdotV;
    float NdotH;
    float LdotH;
    float VdotH; 
};

struct PixelData {
    vec3  baseColor;
    vec3  F0;
    vec3  F90;
    vec2  uv;
    float perceptualRoughness;
    float alphaRoughness;
    float metallic;
    float ambientOcclusion;
};

struct SHADER_ALIGN DrawData {
    float lodTarget;
    uint  drawCount;
};

struct SHADER_ALIGN MeshTaskCommand {
    uint drawId;
    uint taskOffset;
    uint taskCount;
    uint visibilityOffset;
};

struct MeshTaskPayload {
    uint clusterIndices[TASK_GROUP_SIZE];
};

struct SHADER_ALIGN Instance {
    mat4  world;
    mat4  normalMatrix;
    float scale;
    uint  mesh;
    uint  baseTexture;
    uint  metalnessTexture;
    uint  normalTexture;
    uint  transparency;
    uint  visibilityOffset;
};

struct MeshLod {
    uint  meshletOffset;
    uint  meshletCount;
    float lodError;
};

struct SimpleMesh {
    uint vertexOffset;
    uint primitiveOffset;
    uint vertexCount;
    uint primitiveCount;
};

#if (defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)) || defined(__cplusplus)
struct VertexCompressed {
    float16_t px, py, pz;
    uint8_t   nx, ny, nz;
    uint8_t   tx, ty, tz;
    int8_t    ts;
    float16_t tu, tv;
};

struct MeshletCompressed {
    float16_t center[3], radius;
    int8_t    coneAxis[3], coneCutoff;
    uint      vertexOffset;
    uint      primitiveOffset;
    uint16_t  vertexCount;
    uint16_t  primitiveCount;
};

struct MeshCompressed {
    float16_t center[3], radius;
    float16_t bboxMin[3];
    float16_t bboxMax[3];
    uint8_t   lodCount;
    MeshLod   lods[8];
};

#ifndef __cplusplus
#define Vertex VertexCompressed
#define Meshlet MeshletCompressed
#define Mesh MeshCompressed
#endif
#endif

#if !defined(SHADER_8BIT_STORAGE_SUPPORTED) || !defined(SHADER_16BIT_STORAGE_SUPPORTED) || defined(__cplusplus)
struct VertexNonCompressed {
    float px, py, pz;
    float nx, ny, nz;
    float tx, ty, tz;
    float ts;
    float tu, tv;
};

struct MeshletNonCompressed {
    float center[3], radius;
    float coneAxis[3], coneCutoff;
    uint  vertexOffset;
    uint  primitiveOffset;
    uint  vertexCount;
    uint  primitiveCount;
};

struct MeshNonCompressed {
    float   center[3], radius;
    float   bboxMin[3];
    float   bboxMax[3];
    uint    lodCount;
    MeshLod lods[8];
};

#ifndef __cplusplus
#define Vertex VertexNonCompressed
#define Meshlet MeshletNonCompressed
#define Mesh MeshNonCompressed
#endif
#endif

struct IndexedIndirectDraw {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int  vertexOffset;
    uint firstInstance;
};

#endif
