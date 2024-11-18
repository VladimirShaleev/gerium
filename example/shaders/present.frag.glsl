#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0, set = 0) uniform sampler2D texColor;

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x + b)) / (x*(c*x + d) + e), vec3(0.0), vec3(1.0));
}

void main() {
    vec3 color = textureLod(texColor, texCoord, 0).rgb;

    float exposure = 1.0;

    color = ACESFilm(color * exposure);
    color = pow(color, vec3(1.0 / 2.2)); 

    outColor = vec4(color, 1.0);
}
