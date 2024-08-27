#ifndef BVHNODE_HPP
#define BVHNODE_HPP

#include "Model.hpp"

class Scene;

class BVHNode final {
public:
    ~BVHNode() noexcept;

    BVHNode(const BVHNode&) = delete;
    BVHNode(BVHNode&&) = delete;

    BVHNode& operator=(const BVHNode&) = delete;
    BVHNode& operator=(BVHNode&&) = delete;

    static BVHNode* build(Scene& scene);

    bool leaf() const noexcept {
        return _leaf;
    }

    const BVHNode* left() const noexcept {
        return _left;
    }

    const BVHNode* right() const noexcept {
        return _right;
    }

    const BoundingBox& bbox() const noexcept {
        return _bbox;
    }

    const std::vector<Mesh*>& meshes() const noexcept {
        return _meshes;
    }

private:
    BVHNode(
        bool leaf,
        const BoundingBox& bbox,
        const std::vector<Mesh*>& meshes,
        BVHNode* left = nullptr,
        BVHNode* right = nullptr)
        :
        _leaf(leaf),
        _bbox(bbox),
        _left(left),
        _right(right),
        _parent(nullptr),
        _meshes(meshes),
        _depth(-1) {
    }

    void setParent(BVHNode* parent) noexcept {
        _parent = parent;
    }

    static BVHNode* build(const std::vector<Mesh*>& meshes, unsigned depth);

    static BVHNode* buildLeaf(const std::vector<Mesh*>& meshes);

    static BoundingBox bboxFromMeshes(const std::vector<Mesh*>& meshes) noexcept;

    bool _leaf{};
    BoundingBox _bbox{};
    BVHNode* _left{};
    BVHNode* _right{};
    BVHNode* _parent{};
    std::vector<Mesh*> _meshes;
    mutable int _depth;
};

#endif
