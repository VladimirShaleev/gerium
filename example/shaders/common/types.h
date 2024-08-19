#ifndef TYPES_H
#define TYPES_H

struct SceneData {
    mat4 viewProjection;
    vec4 eye;
};

struct MeshData {
    mat4 world;
    mat4 inverseWorld;
};

#endif
