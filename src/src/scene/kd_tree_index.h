#ifndef S21_KD_TREE_INDEX_H
#define S21_KD_TREE_INDEX_H

#include "common/spatial_index.h"
#include <memory>
#include <vector>

namespace s21 {
class KdTreeMeshIndex : public ISpatialIndex {
public:
    void Build(const std::vector<BoundingBox>& boxes) override;
    void Clear() override;
    std::vector<size_t> QueryRay(const Point& origin,
                                 const Point& direction) const override;
private:
    struct Node {
        BoundingBox bbox;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        std::vector<size_t> indices;
        int splitAxis = -1;
        float splitPos = 0.0f;
        bool IsLeaf() const { return indices.empty() == false; }
    };
    std::unique_ptr<Node> BuildRecursive(std::vector<std::pair<BoundingBox, size_t>> items, int depth);
    void QueryRecursive(const Node* node, const Point& origin, const Point& direction, std::vector<size_t>& result) const;
    std::unique_ptr<Node> root_;
    std::vector<BoundingBox> originalBoxes_;
};
}  // namespace s21
#endif  // S21_KD_TREE_INDEX_H
