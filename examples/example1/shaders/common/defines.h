// This file contains common defines and constants used across the rendering pipeline,
// including descriptor set bindings, limits, and shader-specific configurations.
#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

// Descriptor set indices for organizing GPU resources.
#define SCENE_DATA_SET     0
#define CLUSTER_DATA_SET   1
#define INSTANCES_DATA_SET 2
#define TEXTURES_SET       3

// Binding point for the bindless texture array (if supported).
#define BINDLESS_BINDING 10

// Limits for resources in the rendering pipeline.
#define MAX_DYNAMIC_INSTANCES       100
#define MAX_DYNAMIC_MATERIALS       100
#define MAX_TECHNIQUES              50
#define MAX_INSTANCES_PER_TECHNIQUE (MAX_DYNAMIC_INSTANCES + 400)

#define PI 3.1415926538

// Alignment for GPU-friendly data structures.
// In C++, data is aligned to 16 bytes to match GPU requirements.
// In GLSL, no explicit alignment is needed.
#ifdef __cplusplus
// Align structures to 16 bytes for GPU compatibility
#define SHADER_ALIGN alignas(16)
#else
// No alignment needed in GLSL
#define SHADER_ALIGN
#endif

// Use the GLM (OpenGL Mathematics) namespace in C++ for vector and matrix operations.
#ifdef __cplusplus
using namespace glm;
#endif

// Define 16-bit floating-point types for use in C++.
// These types are used for compressed data storage when supported.
#ifdef __cplusplus
#define float16_t u16
#define f16vec2   u16vec2
#define f16vec3   u16vec3
#define f16vec4   u16vec4
#endif

// Enable compressed data types if 8-bit and 16-bit storage extensions are supported in GLSL.
// This allows for more efficient memory usage on GPUs that support these features.
#if defined(SHADER_8BIT_STORAGE_SUPPORTED) && defined(SHADER_16BIT_STORAGE_SUPPORTED)
#extension GL_EXT_shader_16bit_storage : require
#extension GL_EXT_shader_8bit_storage : require
// Define this macro to indicate compressed types are enabled
#define USE_COMPRESSED_TYPES
#elif defined(__cplusplus)
// Enable compressed types in C++ (e.g., for CPU-side processing)
#define USE_COMPRESSED_TYPES
#endif

#endif
