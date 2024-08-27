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
    std::vector<Model*> models;
    models.resize(1000);
    gerium_uint16_t modelCount = 1000;
    scene.getComponents<Model>(modelCount, models.data());

    std::vector<Mesh*> meshes;
    for (gerium_uint16_t i = 0; i < modelCount; ++i) {
        auto model = models[i];
        for (auto& mesh : model->meshes()) {
            meshes.push_back(&mesh);
        }
    }
    return build(meshes, 0);
}

BVHNode* BVHNode::build(const std::vector<Mesh*>& meshes, unsigned depth) {
    if (depth > 60 || meshes.size() <= 1) {
        return buildLeaf(meshes);
    }

    auto minCost      = std::numeric_limits<float>::max();
    auto bestAxis     = Axis::NONE;
    auto bestSplitPos = 0.0f;

    auto bbox = bboxFromMeshes(meshes);
    
    auto numBins = std::max(32u, unsigned(1024.0f / std::powf(2.0f, float (depth))));
    auto invNumBins = 1.0f / numBins;

    for (auto axis : { Axis::X, Axis::Y, Axis::Z }) {
        auto bbAxisWidth = bbox.getWidth(axis);
        auto bbAxisStart = bbox.getAxisStart(axis);
        if (bbAxisWidth < 1e-6) continue;

        auto stepSize = bbAxisWidth * invNumBins;
        auto invStepSize = 1.0f / stepSize;
        std::unique_ptr<BoundingBox[]> binBBs(new BoundingBox[numBins]);
        std::unique_ptr<unsigned[]> binCounts(new unsigned[numBins]);
        for (auto i = 0u; i < numBins; ++i) {
            binCounts[i] = 0;
        }

        for (const auto& mesh : meshes) {
            auto center = mesh->worldBoundingBox().getCenter(axis);
            auto bin = std::min(numBins - 1, unsigned(std::floorf((center - bbAxisStart) * invStepSize)));
            binBBs[bin] = binBBs[bin].combine(mesh->worldBoundingBox());
            ++binCounts[bin];
        }
        for (auto i = 0u; i < numBins - 1; ++i) {
            BoundingBox leftBB;
            BoundingBox rightBB;
            unsigned leftCount = 0;
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
                minCost = partitionCost;
                bestAxis = axis;
                bestSplitPos = bbAxisStart + (i + 1) * stepSize;
            }
        }
    }

    if (bestAxis == Axis::NONE) {
        return buildLeaf(meshes);
    }

    std::vector<Mesh*> leftMeshes;
    std::vector<Mesh*> rightMeshes;

    for (const auto& mesh : meshes) {
        if (mesh->worldBoundingBox().getCenter(bestAxis) < bestSplitPos) {
            leftMeshes.push_back(mesh);
        } else {
            rightMeshes.push_back(mesh);
        }
    }

    if (leftMeshes.size() == meshes.size() || rightMeshes.size() == meshes.size()) {
        return buildLeaf(meshes);
    }
    
    auto innerNode = new BVHNode(false, bbox, {}, build(leftMeshes, depth + 1), build(rightMeshes, depth + 1));
    innerNode->_left->_parent = innerNode;
    innerNode->_right->_parent = innerNode;

    return innerNode;
}

BVHNode* BVHNode::buildLeaf(const std::vector<Mesh*>& meshes) {
    return new BVHNode(true, bboxFromMeshes(meshes), meshes);
}

BoundingBox BVHNode::bboxFromMeshes(const std::vector<Mesh*>& meshes) noexcept {
    BoundingBox bbox;
    for (int i = 0; i < meshes.size(); ++i) {
        bbox = bbox.combine(meshes[i]->worldBoundingBox());
    }
    return bbox;
}
