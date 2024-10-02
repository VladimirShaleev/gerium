#version 450

#include "common/types.h"

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform sampler2D texLight;

void main() {
    outColor = textureLod(texLight, texCoord, 0);
}
