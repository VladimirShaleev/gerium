#version 450

layout(location = 0) in vec2 texCoord;

layout(binding = 0) uniform sampler2D texAlbedo;
layout(binding = 1) uniform sampler2D texNormal;
layout(binding = 2) uniform sampler2D texMetallicRoughness;
layout(binding = 3) uniform sampler2D texVelocity;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texAlbedo, texCoord);
}
