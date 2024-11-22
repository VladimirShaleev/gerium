#version 450

#include "common/types.h"

layout(triangles, invocations = CSM_MAX_CASCADES) in;
layout(triangle_strip, max_vertices = 3) out;

layout (std140, binding = 0, set = 1) uniform LightSpaceMatricesUBO {
    mat4 lightSpaceMatrices[CSM_MAX_CASCADES];
};

void main() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = lightSpaceMatrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
