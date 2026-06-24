#ifndef S21_SPATIAL_INDEX_H
#define S21_SPATIAL_INDEX_H

#include <vector>
#include "common/point.h"

namespace s21 {

struct BoundingBox;   // предварительное объявление (полное определение в scene/mesh.h)

class ISpatialIndex {
 public:
  virtual ~ISpatialIndex() = default;
  virtual void Build(const std::vector<BoundingBox>& boxes) = 0;
  virtual void Clear() = 0;
  virtual bool IsBuilt() const = 0;                     // добавили метод
  virtual std::vector<size_t> QueryRay(const Point& origin,
                                       const Point& direction) const = 0;
};

}  // namespace s21

#endif  // S21_SPATIAL_INDEX_H
