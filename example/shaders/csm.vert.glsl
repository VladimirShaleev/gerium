#version 450

#include "common/types.h"

layout(std140, binding = 0, set = SCENE_DATA_SET) uniform Scene {
    SceneData scene;
};

void main() {
}
