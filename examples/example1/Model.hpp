#ifndef MODEL_HPP
#define MODEL_HPP

#include "Common.hpp"
#include "components/Renderable.hpp"

struct Cluster {
    std::vector<VertexNonCompressed> vertices;
    std::vector<MeshNonCompressed> meshes;
    std::vector<uint32_t> primitiveIndices;
};

struct Model : private NonCopyable {
    struct Node {
        gerium_sint32_t parent : 24;
        gerium_sint32_t level  : 8;
        entt::hashed_string name;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    struct Mesh {
        gerium_uint32_t meshIndex;
        gerium_uint32_t materialIndex;
        gerium_sint32_t nodeIndex;
    };

    std::vector<char> strPool;
    entt::hashed_string name;
    std::vector<Node> nodes{};
    std::vector<Mesh> meshes{};
    std::vector<MaterialData> materials{};
};

Model loadModel(Cluster& cluster, const entt::hashed_string& name);

#endif
