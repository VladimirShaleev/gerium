#version 450

layout(location = 0) out vec2 uv;

layout(std140, binding = 0, set = 0) uniform LightCountUBO {
    mat4 rotate;
};

void main() {
    uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = rotate * vec4(uv * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
}
