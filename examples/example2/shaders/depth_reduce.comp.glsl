/*
 * This code is based on the Niagara project https://github.com/zeux/niagara
 *
 *    MIT License
 * 
 *    Copyright (c) 2018 Arseny Kapoulkine
 *
 *    https://github.com/zeux/niagara/blob/6e1f3f5f5a21363b328e251377bfdf0093b6b405/src/shaders/depthreduce.comp.glsl
 */

#version 450

#include "common/defines.h"

layout(set = 0, binding = 0) uniform sampler2D depth;

layout(set = 0, binding = 1, r32f) uniform writeonly image2D reduce;

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(std140, binding = 2, set = SCENE_DATA_SET) uniform ImageSize {
    vec2 imageSize;
};

void main() {
    uvec2 pos = ivec2(gl_GlobalInvocationID.xy);

    float result = texture(depth, (vec2(pos) + vec2(0.5)) / imageSize).r;

    imageStore(reduce, ivec2(pos), vec4(result));
}
