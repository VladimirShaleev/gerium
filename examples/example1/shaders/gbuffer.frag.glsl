#version 450

#extension GL_EXT_nonuniform_qualifier : enable

#include "common/types.h"

layout(location = 0) in vec2 texcoord;
layout(location = 1) flat in uint material;

layout(location = 0) out vec4 outBase;

layout(std430, binding = 1, set = INSTANCES_DATA_SET) readonly buffer MaterialSSBO {
    Material materials[];
};

layout(binding = BINDLESS_BINDING, set = TEXTURES_SET) uniform sampler2D globalTextures[];

void main() {
    uint baseColorHandle = uint(materials[material].baseColorTexture);

    vec4 baseColor = texture(globalTextures[nonuniformEXT(baseColorHandle)], texcoord);

    outBase = baseColor * COLOR;
}
