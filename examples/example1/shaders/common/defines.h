#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#define SCENE_DATA_SET     0
#define CLUSTER_DATA_SET   1
#define INSTANCES_DATA_SET 2

#define PI 3.1415926538

#ifdef __cplusplus
#define SHADER_ALIGN alignas(16)
#else
#define SHADER_ALIGN
#endif

#ifdef __cplusplus
using namespace glm;
#endif

#ifdef __cplusplus
#define float16_t u16
#define f16vec2   u16vec2
#define f16vec3   u16vec3
#define f16vec4   u16vec4
#endif

#if defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_8bit_storage : require
#define USE_COMPRESSED_TYPES
#elif defined(__cplusplus)
#define USE_COMPRESSED_TYPES
#endif

#endif
