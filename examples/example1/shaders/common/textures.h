// This file defines texture sampling functionality for shaders, supporting both bindless and traditional texture
// binding. Bindless textures allow for more flexible and efficient texture access, while traditional binding is a
// fallback for older hardware.

#ifndef COMMON_TEXTURES_H
#define COMMON_TEXTURES_H

#include "defines.h"

// If bindless textures are supported, enable the necessary GLSL extension and define a bindless texture array.
#ifdef BINDLESS_SUPPORTED

// Enable the GL_EXT_nonuniform_qualifier extension for bindless texture support.
#extension GL_EXT_nonuniform_qualifier : enable

// Define a bindless texture array that can hold all textures in the scene.
// This array is bound to a specific binding point and descriptor set.
layout(binding = BINDLESS_BINDING, set = TEXTURES_SET) uniform sampler2D globalTextures[];

// Macro to fetch a color from a texture using its handle and texture coordinates.
// The `nonuniformEXT` qualifier is used to avoid undefined behavior when accessing textures dynamically.
#define fetchColor(sampler, handle, texcoord) texture(globalTextures[nonuniformEXT(uint(handle))], texcoord)

#else

// If bindless textures are not supported, use traditional texture binding.
// Each texture type is bound to a specific binding point in the TEXTURES_SET descriptor set.
layout(binding = 0, set = TEXTURES_SET) uniform sampler2D baseColorTexture;
layout(binding = 1, set = TEXTURES_SET) uniform sampler2D metallicRoughnessTexture;
layout(binding = 2, set = TEXTURES_SET) uniform sampler2D normalTexture;
layout(binding = 3, set = TEXTURES_SET) uniform sampler2D occlusionTexture;
layout(binding = 4, set = TEXTURES_SET) uniform sampler2D emissiveTexture;

// Macro to fetch a color from a texture using traditional binding.
// Since textures are explicitly bound, the `handle` parameter is ignored.
#define fetchColor(sampler, handle, texcoord) texture(sampler, texcoord)

#endif

// Macros to simplify fetching specific texture types (base color, metallic-roughness, etc.).
// These macros use the `fetchColor` macro internally and provide a consistent interface regardless of bindless support.

// Fetch the base color texture for a given material and texture coordinates.
#define fetchBaseColor(material, texcoord) fetchColor(baseColorTexture, (material).baseColorTexture, (texcoord))

// Fetch the metallic-roughness texture for a given material and texture coordinates.
#define fetchMetallicRoughness(material, texcoord) fetchColor(metallicRoughnessTexture, (material).metallicRoughnessTexture, (texcoord))

// Fetch the normal map texture for a given material and texture coordinates.
#define fetchNormal(material, texcoord) fetchColor(normalTexture, (material).normalTexture, (texcoord))

// Fetch the occlusion texture for a given material and texture coordinates.
#define fetchOcclusion(material, texcoord) fetchColor(occlusionTexture, (material).occlusionTexture, (texcoord))

// Fetch the emissive texture for a given material and texture coordinates.
#define fetchEmissive(material, texcoord) fetchColor(emissiveTexture, (material).emissiveTexture, (texcoord))

#endif
