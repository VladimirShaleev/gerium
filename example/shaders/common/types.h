#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

struct SceneData {
    GLM_NAMESPACE mat4 viewProjection;
    GLM_NAMESPACE mat4 prevViewProjection;
    GLM_NAMESPACE vec4 eye;
    GLM_NAMESPACE vec2 jitter;
    GLM_NAMESPACE vec2 prevJitter;
    GLM_NAMESPACE vec2 invResolution;
    GLM_NAMESPACE ivec2 resolution;
};

struct MeshData {
    GLM_NAMESPACE mat4 world;
    GLM_NAMESPACE mat4 inverseWorld;
    GLM_NAMESPACE mat4 prevWorld;
#ifdef BINDLESS_SUPPORTED
    GLM_NAMESPACE uvec4 textures;
#endif
};

#endif
