#version 450

#include "common/types.h"

layout(location = 0) in vec4 position;

layout(std430, binding = 5, set = 0) readonly buffer Instances {
    Instance instances[];
};

void main() {
    gl_Position = instances[gl_InstanceIndex].world * position;
}
