#version 450

layout(binding = 1, set = 0) uniform sampler2D texSampler;

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = textureLod(texSampler, uv, 0);
}
