#version 450

#include "common/types.h"

layout(location = 0) out vec4 dir;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

void main() {
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    
    gl_Position = vec4(uv.xy * 2.0 - 1.0, 0.0, 1.0);
    gl_Position.y = -gl_Position.y;

    dir = scene.invViewProjection * gl_Position;
}
