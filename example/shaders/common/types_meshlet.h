#ifndef COMMON_TYPES_MESHLET_H
#define COMMON_TYPES_MESHLET_H

#include "defines.h"

#ifndef __cplusplus
#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage:  require
#endif

#ifdef __cplusplus
#define f16vec2 u16vec2
#endif

struct Vertex {
    GLM_NAMESPACE vec4 position;
    GLM_NAMESPACE u8vec4 normal;
    GLM_NAMESPACE f16vec2 texcoord;
    GLM_NAMESPACE uint _pad0;
    GLM_NAMESPACE uint _pad1;
};

#endif
