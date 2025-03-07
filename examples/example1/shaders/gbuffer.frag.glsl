#version 450

#include "common/types.h"
#include "common/textures.h"

layout(location = 0) in vec2 texcoord;
layout(location = 1) flat in uint material;

layout(location = 0) out vec4 outBase;

layout(std430, binding = 1, set = INSTANCES_DATA_SET) readonly buffer MaterialSSBO {
    Material materials[];
};

void main() {
    vec4 baseColor = fetchBaseColor(materials[material], texcoord);

    outBase = baseColor * COLOR;
}
