#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "defines.h"

struct SceneData {
    GLM_NAMESPACE mat4 viewProjection;
    GLM_NAMESPACE vec4 eye;
};

struct MeshData {
    GLM_NAMESPACE mat4 world;
    GLM_NAMESPACE mat4 inverseWorld;
#ifdef BINDLESS_SUPPORTED
    GLM_NAMESPACE uvec4 textures;
#endif
};

#endif
