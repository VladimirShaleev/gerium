#ifndef COMMON_TEXTURES_H
#define COMMON_TEXTURES_H

#include "defines.h"

#ifdef BINDLESS_SUPPORTED
#extension GL_EXT_nonuniform_qualifier : enable
layout(binding = BINDLESS_BINDING, set = TEXTURE_SET) uniform sampler2D globalTextures[];

#define definePbrTextures()

#define fetchColor(sampler, handle, texcoord) \
    texture(globalTextures[nonuniformEXT(handle)], texcoord)

#else

#define definePbrTextures() \
    layout(binding = 0, set = TEXTURE_SET) uniform sampler2D baseColor; \
    layout(binding = 1, set = TEXTURE_SET) uniform sampler2D normalColor; \
    layout(binding = 2, set = TEXTURE_SET) uniform sampler2D metallicRoughnessColor;

#define fetchColor(sampler, handle, texcoord) \
    texture(sampler, texcoord)

#endif

#define fetchBase(instanceID, texcoord) fetchColor(baseColor, mesh[instanceID].textures.x, texcoord)
#define fetchNormal(instanceID, texcoord) fetchColor(normalColor, mesh[instanceID].textures.y, texcoord)
#define fetchMetallicRoughness(instanceID, texcoord) fetchColor(metallicRoughnessColor, mesh[instanceID].textures.z, texcoord)

#endif
