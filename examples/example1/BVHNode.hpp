#ifndef BVHNODE_HPP
#define BVHNODE_HPP

#include "Model.hpp"

class Scene;
class Light;

class BVHNode final {
public:
    enum Type {
        MeshType,
        LightType
    };

    struct Object {
        Type type;
        union {
            Mesh* mesh;
            Light* light;
        };
    };

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

    const std::vector<Object>& objects() const noexcept {
        return _objects;
    }

private:
    BVHNode(
        bool leaf,
        const BoundingBox& bbox,
        const std::vector<Object>& objects,
        BVHNode* left = nullptr,
        BVHNode* right = nullptr)
        :
        _leaf(leaf),
        _bbox(bbox),
        _left(left),
        _right(right),
        _parent(nullptr),
        _objects(objects),
        _depth(-1) {
    }

    void setParent(BVHNode* parent) noexcept {
        _parent = parent;
    }

    static BVHNode* build(const std::vector<Object>& objects, unsigned depth);

    static BVHNode* buildLeaf(const std::vector<Object>& objects);

    static BoundingBox bboxFromObjects(const std::vector<Object>& objects) noexcept;

    bool _leaf{};
    BoundingBox _bbox{};
    BVHNode* _left{};
    BVHNode* _right{};
    BVHNode* _parent{};
    std::vector<Object> _objects;
    mutable int _depth;
};

#endif
