#ifndef S21_MESH_H
#define S21_MESH_H

#include <cstddef>
#include <vector>
#include <algorithm>    // для std::min, std::max
#include <limits>       // для std::numeric_limits

#include "common/edge.h"
#include "common/point.h"
#include "common/model_data.h"
#include "scene_object.h"

namespace s21 {

struct BoundingSphere {
  Point center;
  float radius;
};

struct BoundingBox {
  Point min;
  Point max;

  [[nodiscard]] bool IntersectsRay(const Point& origin,
                                   const Point& direction) const {
    float tmin = -std::numeric_limits<float>::infinity();
    float tmax =  std::numeric_limits<float>::infinity();

    // Ось X
    if (std::abs(direction.x) > 1e-8f) {
      float tx1 = (min.x - origin.x) / direction.x;
      float tx2 = (max.x - origin.x) / direction.x;
      tmin = std::max(tmin, std::min(tx1, tx2));
      tmax = std::min(tmax, std::max(tx1, tx2));
    } else {
      if (origin.x < min.x || origin.x > max.x) return false;
    }

    // Ось Y
    if (std::abs(direction.y) > 1e-8f) {
      float ty1 = (min.y - origin.y) / direction.y;
      float ty2 = (max.y - origin.y) / direction.y;
      tmin = std::max(tmin, std::min(ty1, ty2));
      tmax = std::min(tmax, std::max(ty1, ty2));
    } else {
      if (origin.y < min.y || origin.y > max.y) return false;
    }

    // Ось Z
    if (std::abs(direction.z) > 1e-8f) {
      float tz1 = (min.z - origin.z) / direction.z;
      float tz2 = (max.z - origin.z) / direction.z;
      tmin = std::max(tmin, std::min(tz1, tz2));
      tmax = std::min(tmax, std::max(tz1, tz2));
    } else {
      if (origin.z < min.z || origin.z > max.z) return false;
    }

    return tmax >= std::max(tmin, 0.0f);
  }
};

class Mesh : public SceneObject {
 public:
  void AddVertex(const Point& vertex);
  void AddEdge(const Edge& edge);

  void SetVertices(const std::vector<Point>& vertices);
  void SetEdges(const std::vector<Edge>& edges);
  void SetNormals(const std::vector<Point>& normals);
  void SetUVs(const std::vector<Point2D>& uvs);
  void SetTriangles(const std::vector<unsigned int>& triangles);
  void SetFlatNormals(const std::vector<Point>& flat_normals);
  void SetSmoothNormals(const std::vector<Point>& smooth_normals);

  const std::vector<Point>& GetVertices() const;
  const std::vector<Edge>& GetEdges() const;
  const std::vector<Point>& GetNormals() const;
  const std::vector<Point2D>& GetUVs() const;
  const std::vector<unsigned int>& GetTriangles() const;
  const std::vector<Point>& GetFlatNormals() const;
  const std::vector<Point>& GetSmoothNormals() const;

  size_t VertexCount() const;
  size_t EdgeCount() const;
  size_t TriangleCount() const;

  void Clear();
  void ComputeNormals(float smoothingFactor = 1.0f);
  void ComputeBoundingSphere();
  void OptimizeTriangulation(int max_iterations = 5);

  const BoundingSphere& GetBoundingSphere() const { return bounding_sphere_; }
  const std::string& GetFileName() const { return GetSourceFile(); }

  BoundingBox GetBoundingBox() const;

 private:
  std::vector<Point> vertices_;
  std::vector<Edge> edges_;
  std::vector<Point> normals_;
  std::vector<Point> flat_normals_;
  std::vector<Point> smooth_normals_;
  std::vector<Point2D> uvs_;
  std::vector<unsigned int> triangles_;
  BoundingSphere bounding_sphere_;
};

}  // namespace s21

#endif
