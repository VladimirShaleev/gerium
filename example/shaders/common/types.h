#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#define SCENE_DATA_SET 0

#define MESH_DATA_SET 1

#define TEXTURE_SET 2

#define MAX_INSTANCES 100

#ifdef __cplusplus
# define GLM_NAMESPACE glm::
#else
# define GLM_NAMESPACE 
#endif

struct SceneData {
    GLM_NAMESPACE mat4 viewProjection;
    GLM_NAMESPACE vec4 eye;
};

struct MeshData {
    GLM_NAMESPACE mat4 world;
    GLM_NAMESPACE mat4 inverseWorld;
};

#endif
