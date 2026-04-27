#include "kd_tree_index.h"
#include <algorithm>
#include <limits>

namespace s21 {

void KdTreeMeshIndex::Build(const std::vector<BoundingBox>& boxes) {
    originalBoxes_ = boxes;
    if (boxes.empty()) {
        root_.reset();
        return;
    }

    std::vector<std::pair<BoundingBox, size_t>> items;
    items.reserve(boxes.size());
    for (size_t i = 0; i < boxes.size(); ++i)
        items.emplace_back(boxes[i], i);

    root_ = BuildRecursive(std::move(items), 0);
}

void KdTreeMeshIndex::Clear() {
    root_.reset();
    originalBoxes_.clear();
}

std::unique_ptr<KdTreeMeshIndex::Node>
KdTreeMeshIndex::BuildRecursive(std::vector<std::pair<BoundingBox, size_t>> items,
                                int depth) {
    if (items.empty())
        return nullptr;

    auto node = std::make_unique<Node>();

    Point minP = items[0].first.min;
    Point maxP = items[0].first.max;
    for (const auto& item : items) {
        minP.x = std::min(minP.x, item.first.min.x);
        minP.y = std::min(minP.y, item.first.min.y);
        minP.z = std::min(minP.z, item.first.min.z);
        maxP.x = std::max(maxP.x, item.first.max.x);
        maxP.y = std::max(maxP.y, item.first.max.y);
        maxP.z = std::max(maxP.z, item.first.max.z);
    }
    node->bbox = BoundingBox{minP, maxP};

    const size_t kLeafMaxItems = 4;
    const int kMaxDepth = 20;
    if (items.size() <= kLeafMaxItems || depth >= kMaxDepth) {
        node->indices.reserve(items.size());
        for (const auto& item : items)
            node->indices.push_back(item.second);
        return node;
    }

    std::vector<float> centers[3];
    centers[0].reserve(items.size());
    centers[1].reserve(items.size());
    centers[2].reserve(items.size());
    for (const auto& item : items) {
        Point c = {(item.first.min.x + item.first.max.x) * 0.5f,
                   (item.first.min.y + item.first.max.y) * 0.5f,
                   (item.first.min.z + item.first.max.z) * 0.5f};
        centers[0].push_back(c.x);
        centers[1].push_back(c.y);
        centers[2].push_back(c.z);
    }

    int bestAxis = 0;
    float maxSpread = -1.0f;
    for (int axis = 0; axis < 3; ++axis) {
        auto [minIt, maxIt] = std::minmax_element(centers[axis].begin(), centers[axis].end());
        float spread = *maxIt - *minIt;
        if (spread > maxSpread) {
            maxSpread = spread;
            bestAxis = axis;
        }
    }
    node->splitAxis = bestAxis;

    auto& axisCenters = centers[bestAxis];
    std::nth_element(axisCenters.begin(),
                     axisCenters.begin() + axisCenters.size() / 2,
                     axisCenters.end());
    float median = axisCenters[axisCenters.size() / 2];
    node->splitPos = median;

    std::vector<std::pair<BoundingBox, size_t>> leftItems, rightItems;
    leftItems.reserve(items.size() / 2 + 1);
    rightItems.reserve(items.size() / 2 + 1);

    for (auto& item : items) {
        Point c = {(item.first.min.x + item.first.max.x) * 0.5f,
                   (item.first.min.y + item.first.max.y) * 0.5f,
                   (item.first.min.z + item.first.max.z) * 0.5f};
        float coord = (bestAxis == 0) ? c.x : (bestAxis == 1) ? c.y : c.z;
        if (coord < median)
            leftItems.push_back(std::move(item));
        else
            rightItems.push_back(std::move(item));
    }

    if (leftItems.empty() || rightItems.empty()) {
        node->indices.reserve(items.size());
        for (const auto& item : items)
            node->indices.push_back(item.second);
        return node;
    }

    node->left = BuildRecursive(std::move(leftItems), depth + 1);
    node->right = BuildRecursive(std::move(rightItems), depth + 1);

    return node;
}

std::vector<size_t> KdTreeMeshIndex::QueryRay(const Point& origin,
                                              const Point& direction) const {
    std::vector<size_t> result;
    if (!root_)
        return result;

    result.reserve(32);
    QueryRecursive(root_.get(), origin, direction, result);
    return result;
}

void KdTreeMeshIndex::QueryRecursive(const Node* node,
                                     const Point& origin,
                                     const Point& direction,
                                     std::vector<size_t>& result) const {
    if (!node)
        return;

    if (!node->bbox.IntersectsRay(origin, direction))
        return;

    if (node->IsLeaf()) {
        result.insert(result.end(), node->indices.begin(), node->indices.end());
        return;
    }

    QueryRecursive(node->left.get(), origin, direction, result);
    QueryRecursive(node->right.get(), origin, direction, result);
}

}  // namespace s21
