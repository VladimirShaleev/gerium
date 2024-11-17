#version 450

layout(location = 0) in vec4 dir;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 1) uniform samplerCube texEnv;

void main() {
    outColor = vec4(textureLod(texEnv, dir.xyz, 0).rgb, 1.0);
}
