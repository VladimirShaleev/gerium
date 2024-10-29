#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#define SCENE_DATA_SET  0
#define GLOBAL_DATA_SET 1
#define MESH_DATA_SET   2

#define TASK_GROUP_SIZE 64
#define MESH_GROUP_SIZE 64

#define MESH_MAX_VERTICES   64
#define MESH_MAX_PRIMITIVES 124

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
