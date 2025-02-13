#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

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
    vec2  invResolution;
    ivec2 resolution;
    float lodTarget;
};

struct SHADER_ALIGN MeshInstance {
    mat4  world;
    mat4  prevWorld;
    mat4  normalMatrix;
    float scale;
    uint  mesh;
    uint  technique;
    uint  material;
};

struct MeshLod {
    uint  primitiveOffset;
    uint  primitiveCount;
    float lodError;
};

struct VertexNonCompressed {
    float px, py, pz;
    float nx, ny, nz;
    float tx, ty, tz;
    float ts;
    float tu, tv;
};

struct MeshNonCompressed {
    float   center[3], radius;
    float   bboxMin[3];
    float   bboxMax[3];
    uint    vertexOffset;
    uint    vertexCount;
    uint    lodCount;
    MeshLod lods[8];
};

struct MaterialNonCompressed {
    float baseColorFactor[4];
    float emissiveFactor[3];
    float metallicFactor;
    float roughnessFactor;
    float occlusionStrength;
    float alphaCutoff;
    uint  baseColorTexture;
    uint  metallicRoughnessTexture;
    uint  normalTexture;
    uint  occlusionTexture;
    uint  emissiveTexture;
};

#ifndef USE_COMPRESSED_TYPES

#define Vertex   VertexNonCompressed
#define Mesh     MeshNonCompressed
#define Material MaterialNonCompressed

#else

struct Vertex {
    float16_t px, py, pz;
    uint8_t   nx, ny, nz;
    uint8_t   tx, ty, tz;
    int8_t    ts;
    float16_t tu, tv;
};

struct Mesh {
    float16_t center[3], radius;
    float16_t bboxMin[3];
    float16_t bboxMax[3];
    uint      vertexOffset;
    uint      vertexCount;
    uint8_t   lodCount;
    MeshLod   lods[8];
};

struct Material {
    float16_t baseColorFactor[4];
    float16_t emissiveFactor[3];
    float16_t metallicFactor;
    float16_t roughnessFactor;
    float16_t occlusionStrength;
    float16_t alphaCutoff;
    uint16_t  baseColorTexture;
    uint16_t  metallicRoughnessTexture;
    uint16_t  normalTexture;
    uint16_t  occlusionTexture;
    uint16_t  emissiveTexture;
};

#endif

struct IndirectDraw {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
    uint lodIndex;
};

#endif
