#ifndef MODEL_HPP
#define MODEL_HPP

#include "Common.hpp"

struct Cluster {
    std::vector<VertexNonCompressed> vertices;
    std::vector<MeshNonCompressed> meshes;
    std::vector<uint32_t> primitiveIndices;
};

struct MeshIndex {
    gerium_uint32_t meshIndex;
    gerium_sint32_t nodeIndex;
};

struct Model {
    struct Node {
        gerium_sint32_t parent    : 24;
        gerium_sint32_t level     : 8;
        gerium_sint32_t nameIndex : 16;
        gerium_sint32_t nameLen   : 16;
        entt::hashed_string name;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    std::vector<char> strPool;
    entt::hashed_string name;
    std::vector<Node> nodes{};
    std::vector<MeshIndex> meshes{};
};

Model loadModel(Cluster& cluster, const std::string& filename);

#endif
