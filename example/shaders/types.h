#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "common/types_generic.h"

#if defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)
#include "common/types_meshlet.h"
#endif

struct Meshlet {
    vec4 centerAndRadius;
	i8vec4 coneAxisAndCutoff;

    uint vertexOffset;
    uint primitiveOffset;
    uint16_t vertexCount;
    uint16_t primitiveCount;
};

#endif
