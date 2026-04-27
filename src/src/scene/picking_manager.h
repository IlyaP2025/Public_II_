#ifndef S21_PICKING_MANAGER_H
#define S21_PICKING_MANAGER_H

#include <optional>

#include "common/point.h"
#include "scene/camera.h"
#include "scene/scene.h"

namespace s21 {

struct Ray {
  Point origin;
  Point direction;
};

class PickingManager {
 public:
  static Mesh* PickObject(int mouse_x, int mouse_y, int viewport_width,
                          int viewport_height, const Camera& camera,
                          const Scene& scene);
};

}  // namespace s21

#endif  // S21_PICKING_MANAGER_H
