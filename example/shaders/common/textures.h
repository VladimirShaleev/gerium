#ifndef COMMON_TEXTURES_H
#define COMMON_TEXTURES_H

#include "defines.h"

#ifdef BINDLESS_SUPPORTED
#extension GL_EXT_nonuniform_qualifier : enable
layout(binding = BINDLESS_BINDING, set = TEXTURE_SET) uniform sampler2D globalTextures[];

#define fetchColor(sampler, handle, texcoord) \
    texture(globalTextures[nonuniformEXT(handle)], texcoord)

#else

#define fetchColor(sampler, handle, texcoord) \
    texture(sampler, texcoord)

#endif

#endif
