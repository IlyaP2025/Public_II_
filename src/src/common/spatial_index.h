#ifndef S21_SPATIAL_INDEX_H
#define S21_SPATIAL_INDEX_H

#include <vector>
#include "common/point.h"
#include "scene/mesh.h"  // для BoundingBox

namespace s21 {
class ISpatialIndex {
public:
    virtual ~ISpatialIndex() = default;
    virtual void Build(const std::vector<BoundingBox>& boxes) = 0;
    virtual void Clear() = 0;
    virtual std::vector<size_t> QueryRay(const Point& origin,
                                         const Point& direction) const = 0;
};
}  // namespace s21
#endif  // S21_SPATIAL_INDEX_H
