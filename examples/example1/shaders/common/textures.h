#ifndef COMMON_TEXTURES_H
#define COMMON_TEXTURES_H

#include "defines.h"

#ifdef BINDLESS_SUPPORTED

#extension GL_EXT_nonuniform_qualifier : enable
layout(binding = BINDLESS_BINDING, set = TEXTURES_SET) uniform sampler2D globalTextures[];

#define fetchColor(sampler, handle, texcoord) texture(globalTextures[nonuniformEXT(uint(handle))], texcoord)

#else

layout(binding = 0, set = TEXTURES_SET) uniform sampler2D baseColorTexture;
layout(binding = 1, set = TEXTURES_SET) uniform sampler2D metallicRoughnessTexture;
layout(binding = 2, set = TEXTURES_SET) uniform sampler2D normalTexture;
layout(binding = 3, set = TEXTURES_SET) uniform sampler2D occlusionTexture;
layout(binding = 4, set = TEXTURES_SET) uniform sampler2D emissiveTexture;

#define fetchColor(sampler, handle, texcoord) texture(sampler, texcoord)

#endif

#define fetchBaseColor(material, texcoord)         fetchColor(baseColorTexture,         (material).baseColorTexture,         (texcoord))
#define fetchMetallicRoughness(material, texcoord) fetchColor(metallicRoughnessTexture, (material).metallicRoughnessTexture, (texcoord))
#define fetchNormal(material, texcoord)            fetchColor(normalTexture,            (material).normalTexture,            (texcoord))
#define fetchOcclusion(material, texcoord)         fetchColor(occlusionTexture,         (material).occlusionTexture,         (texcoord))
#define fetchEmissive(material, texcoord)          fetchColor(emissiveTexture,          (material).emissiveTexture,          (texcoord))

#endif
