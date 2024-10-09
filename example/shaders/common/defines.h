#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#define SCENE_DATA_SET 0
#define MESH_DATA_SET  1

#define TASK_GROUP_SIZE 64
#define MESH_GROUP_SIZE 64

#define MESH_MAX_VERTICES   64
#define MESH_MAX_PRIMITIVES 124

#define PI 3.1415926538

#ifdef __cplusplus
# define GLM_NAMESPACE glm::
#else
# define GLM_NAMESPACE 
#endif

#endif
