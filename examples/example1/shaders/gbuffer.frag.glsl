#version 450

#include "common/types.h"

layout(location = 0) out vec4 outBase;

void main() {
    outBase = COLOR;
}
