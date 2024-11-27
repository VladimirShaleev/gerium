#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#define SCENE_DATA_SET   0
#define GLOBAL_DATA_SET  1
#define CLUSTER_DATA_SET 2
#define TEXTURE_SET      3

#define TASK_GROUP_SIZE 64
#define MESH_GROUP_SIZE 64
#define SKY_GROUP_SIZE  8

#define MESH_MAX_VERTICES   64
#define MESH_MAX_PRIMITIVES 124

#define CSM_MAX_CASCADES 4

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT       1
#define LIGHT_TYPE_SPOT        2

#define OUTPUT_FINAL_RESULT      0
#define OUTPUT_DIRECT_LIGHT_ONLY 1
#define OUTPUT_RADIANCE_CACHE    2
#define OUTPUT_IRRADIANCE_CACHE  3
#define OUTPUT_MESHLETS          4
#define OUTPUT_ALBEDO            5
#define OUTPUT_NORMAL            6
#define OUTPUT_METALNESS         7
#define OUTPUT_ROUGHNESS         8
#define OUTPUT_MOTION            9

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

#endif
