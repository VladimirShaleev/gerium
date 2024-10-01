#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#define SCENE_DATA_SET 0
#define MESH_DATA_SET  1
#define TEXTURE_SET    2

#define BINDLESS_BINDING 10

#define MAX_INSTANCES 100

#define MAX_LIGHTS   256
#define LIGHT_Z_BINS 24
#define TILE_SIZE    12
#define NUM_WORDS    ((MAX_LIGHTS + 31) / 32)
#define BIN_WIDTH    (1.0f / LIGHT_Z_BINS)

#define PI 3.1415926538

#ifdef __cplusplus
# define GLM_NAMESPACE glm::
#else
# define GLM_NAMESPACE 
#endif

#endif
