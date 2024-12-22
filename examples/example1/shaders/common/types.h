#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

struct SceneData {
    GLM_NAMESPACE mat4 view;
    GLM_NAMESPACE mat4 viewProjection;
    GLM_NAMESPACE mat4 prevViewProjection;
    GLM_NAMESPACE mat4 invViewProjection;
    GLM_NAMESPACE vec4 viewPosition;
    GLM_NAMESPACE vec4 eye;
    GLM_NAMESPACE vec2 planes;
    GLM_NAMESPACE vec2 jitter;
    GLM_NAMESPACE vec2 prevJitter;
    GLM_NAMESPACE vec2 invResolution;
    GLM_NAMESPACE ivec2 resolution;
    GLM_NAMESPACE ivec2 pad0;
};

struct MeshData {
    GLM_NAMESPACE mat4 world;
    GLM_NAMESPACE mat4 inverseWorld;
    GLM_NAMESPACE mat4 prevWorld;
#ifdef BINDLESS_SUPPORTED
    GLM_NAMESPACE uvec4 textures;
#endif
};

struct PointLight {
    GLM_NAMESPACE vec4 position;
    GLM_NAMESPACE vec4 color;
    float attenuation;
    float intensity;
    float p1;
    float p2;
};

#endif
