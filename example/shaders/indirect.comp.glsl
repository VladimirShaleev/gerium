/*
 * This code is based on the Niagara project https://github.com/zeux/niagara
 *
 *    MIT License
 * 
 *    Copyright (c) 2018 Arseny Kapoulkine
 *
 *    https://github.com/zeux/niagara/blob/6e1f3f5f5a21363b328e251377bfdf0093b6b405/src/shaders/tasksubmit.comp.glsl
 */

#version 450

#include "common/types.h"

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(std430, binding = 0, set = SCENE_DATA_SET) buffer CommandCount {
    uint commandCount;
    uint groupCountX;
    uint groupCountY;
    uint groupCountZ;
};

layout(std430, binding = 1, set = SCENE_DATA_SET) writeonly buffer Commands {
    MeshTaskCommand commands[];
};

void main() {
    uint tid = gl_LocalInvocationID.x;
    uint count = commandCount;

    if (tid == 0) {
        groupCountX = min((count + 63) / 64, 65535);
        groupCountY = 64;
        groupCountZ = 1;
    }

    uint boundary = (count + 63) & ~63;
    MeshTaskCommand dummy;
    dummy.drawId = 0;
    dummy.taskOffset = 0;
    dummy.taskCount = 0;

    if (count + tid < boundary) {
        commands[count + tid] = dummy;
    }
}
