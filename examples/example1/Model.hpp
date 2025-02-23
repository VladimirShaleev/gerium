#ifndef MODEL_HPP
#define MODEL_HPP

#include "Common.hpp"
#include "components/Collider.hpp"
#include "components/Renderable.hpp"

struct MeshCollider {
    std::vector<vec3> vertices;
    std::vector<uint32_t> indices;
};

struct ConvexHullCollider {
    struct ConvexHull {
        std::vector<vec3> vertices;
    };

    std::vector<ConvexHull> convexHulls;
};

struct Cluster {
    std::vector<VertexNonCompressed> vertices;
    std::vector<MeshNonCompressed> meshes;
    std::vector<uint32_t> primitiveIndices;
    std::vector<MeshCollider> meshColliders;
    std::vector<ConvexHullCollider> convexHullColliders;
};

struct Model : private NonCopyable {
    struct Node {
        gerium_sint32_t parent : 24;
        gerium_sint32_t level  : 8;
        gerium_sint32_t colliderIndex;
        Shape colliderShape;
        entt::hashed_string name;
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
        BoundingBox bbox;
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
