#version 450

#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_mesh_shader: require

#include "types.h"

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main() {
	EmitMeshTasksEXT(1510, 1, 1);
}
