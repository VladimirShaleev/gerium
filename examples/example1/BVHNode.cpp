#include "BVHNode.hpp"
#include "Scene.hpp"

BVHNode::~BVHNode() noexcept {
    if (_left) {
        delete _left;
        _left = nullptr;
    }
    if (_right) {
        delete _right;
        _right = nullptr;
    }
}

BVHNode* BVHNode::build(Scene& scene) {
    std::vector<Object> objects;

    std::vector<Model*> models;
    models.resize(1000);
    gerium_uint16_t modelCount = 1000;
    scene.getComponents<Model>(modelCount, models.data());

    for (gerium_uint16_t i = 0; i < modelCount; ++i) {
        auto model = models[i];
        for (auto& mesh : model->meshes()) {
            Object obj;
            obj.type = MeshType;
            obj.mesh = &mesh;
            objects.push_back(obj);
        }
    }

    std::vector<Light*> lights;
    lights.resize(1000);
    gerium_uint16_t lightCount = 1000;
    scene.getComponents<Light>(lightCount, lights.data());

    for (gerium_uint16_t i = 0; i < lightCount; ++i) {
        auto light = lights[i];
        Object obj;
        obj.type  = LightType;
        obj.light = light;
        objects.push_back(obj);
    }
    return build(objects, 0);
}

BVHNode* BVHNode::build(const std::vector<Object>& objects, unsigned depth) {
    if (depth > 60 || objects.size() <= 1) {
        return buildLeaf(objects);
    }

    auto minCost      = std::numeric_limits<float>::max();
    auto bestAxis     = Axis::NONE;
    auto bestSplitPos = 0.0f;

    auto bbox = bboxFromObjects(objects);

    auto numBins    = std::max(32u, unsigned(1024.0f / std::pow(2.0f, float(depth))));
    auto invNumBins = 1.0f / numBins;

    for (auto axis : { Axis::X, Axis::Y, Axis::Z }) {
        auto bbAxisWidth = bbox.getWidth(axis);
        auto bbAxisStart = bbox.getAxisStart(axis);
        if (bbAxisWidth < 1e-6) {
            continue;
        }

        auto stepSize    = bbAxisWidth * invNumBins;
        auto invStepSize = 1.0f / stepSize;
        std::unique_ptr<BoundingBox[]> binBBs(new BoundingBox[numBins]);
        std::unique_ptr<unsigned[]> binCounts(new unsigned[numBins]);
        for (auto i = 0u; i < numBins; ++i) {
            binCounts[i] = 0;
        }

        for (const auto& object : objects) {
            BoundingBox bbox;
            if (object.type == MeshType) {
                bbox = object.mesh->worldBoundingBox();
            } else if (object.type == LightType) {
                bbox = object.light->worldBoundingBox();
            }
            auto center = bbox.getCenter(axis);

            auto bin    = std::min(numBins - 1, unsigned(std::floor((center - bbAxisStart) * invStepSize)));
            binBBs[bin] = binBBs[bin].combine(bbox);
            ++binCounts[bin];
        }
        for (auto i = 0u; i < numBins - 1; ++i) {
            BoundingBox leftBB;
            BoundingBox rightBB;
            unsigned leftCount  = 0;
            unsigned rightCount = 0;

            for (auto j = i + 1; j < numBins; ++j) {
                rightCount += binCounts[j];
                rightBB = rightBB.combine(binBBs[j]);
            }

            for (auto k = 0u; k <= i; ++k) {
                leftCount += binCounts[k];
                leftBB = leftBB.combine(binBBs[k]);
            }

            auto partitionCost = (leftCount * leftBB.sa()) + (rightCount * rightBB.sa());

            if (partitionCost < minCost) {
                minCost      = partitionCost;
                bestAxis     = axis;
                bestSplitPos = bbAxisStart + (i + 1) * stepSize;
            }
        }
    }

    if (bestAxis == Axis::NONE) {
        return buildLeaf(objects);
    }

    std::vector<Object> leftObjects;
    std::vector<Object> rightObjects;

    for (const auto& object : objects) {
        BoundingBox bbox;
        if (object.type == MeshType) {
            bbox = object.mesh->worldBoundingBox();
        } else if (object.type == LightType) {
            bbox = object.light->worldBoundingBox();
        }
        if (bbox.getCenter(bestAxis) < bestSplitPos) {
            leftObjects.push_back(object);
        } else {
            rightObjects.push_back(object);
        }
    }

    if (leftObjects.size() == objects.size() || rightObjects.size() == objects.size()) {
        return buildLeaf(objects);
    }

    auto innerNode = new BVHNode(false, bbox, {}, build(leftObjects, depth + 1), build(rightObjects, depth + 1));
    innerNode->_left->_parent  = innerNode;
    innerNode->_right->_parent = innerNode;

    return innerNode;
}

BVHNode* BVHNode::buildLeaf(const std::vector<Object>& objects) {
    return new BVHNode(true, bboxFromObjects(objects), objects);
}

BoundingBox BVHNode::bboxFromObjects(const std::vector<Object>& objects) noexcept {
    BoundingBox bbox;
    for (int i = 0; i < objects.size(); ++i) {
        if (objects[i].type == MeshType) {
            bbox = bbox.combine(objects[i].mesh->worldBoundingBox());
        } else if (objects[i].type == LightType) {
            bbox = bbox.combine(objects[i].light->worldBoundingBox());
        }
    }
    return bbox;
}
