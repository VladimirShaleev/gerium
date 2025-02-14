#version 450

#include "common/types.h"

layout(location = 0) flat in uint material;

layout(location = 0) out vec4 outBase;

layout(std430, binding = 1, set = INSTANCES_DATA_SET) readonly buffer MaterialSSBO {
    Material materials[];
};

void main() {
    outBase = COLOR;
}
