#ifndef COMMON_TYPES_GENERIC_H
#define COMMON_TYPES_GENERIC_H

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

#endif
