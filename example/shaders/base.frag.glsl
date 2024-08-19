#version 450

layout(location = 0) in vec2 inTexcoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1, set = 1) uniform sampler2D baseColor;
layout(binding = 2, set = 1) uniform sampler2D normalColor;
layout(binding = 3, set = 1) uniform sampler2D metallicRoughnessColor;

void main() {
    outColor = texture(baseColor, inTexcoord);
}
