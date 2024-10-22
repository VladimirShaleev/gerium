#version 450

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(color.rgb, 0.7);
}
