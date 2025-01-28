#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {
    uvec2 v = uvec2(texCoord * vec2(5.0));
    outColor = vec4(vec2(v) / vec2(5.0), 1.0, 1.0);
}
