#version 450

#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_mesh_shader: require

#include "types.h"

struct Meshlet {
	vec3 center;
	float radius;
	int8_t cone_axis[3];
	int8_t cone_cutoff;

	uint dataOffset;
	uint8_t vertexCount;
	uint8_t triangleCount;
};

layout(local_size_x = TASK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform SceneDataUBO {
    SceneData scene;
};

layout(std140, binding = 0, set = 1) readonly buffer Meshlets {
	Meshlet meshlets[];
};

void main() {
    
}
