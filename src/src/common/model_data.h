#ifndef S21_MODEL_DATA_H
#define S21_MODEL_DATA_H

#include <string>
#include <vector>

#include "common/edge.h"
#include "common/point.h"

namespace s21 {

struct RawModelData {
  std::vector<Point> vertices;
  std::vector<Point> normals;  // интерполированные нормали (результат)
  std::vector<Point> flat_normals;  // плоские (per-face)
  std::vector<Point>
      smooth_normals;  // гладкие (усреднённые по исходным вершинам)
  std::vector<Point2D> uvs;
  std::vector<Edge> edges;
  std::string filename;
  std::string errorMsg;
  std::vector<unsigned int> triangles;
  bool success = false;
  float smoothingFactor = 1.0f;
};

}  // namespace s21

#endif
