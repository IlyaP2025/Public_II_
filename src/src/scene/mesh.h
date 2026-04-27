#ifndef S21_MESH_H
#define S21_MESH_H

#include <cstddef>
#include <vector>

#include "common/edge.h"
#include "common/model_data.h"
#include "common/point.h"
#include "scene_object.h"

namespace s21 {

struct BoundingSphere {
  Point center;
  float radius;
};

struct BoundingBox {
  Point min;
  Point max;
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
