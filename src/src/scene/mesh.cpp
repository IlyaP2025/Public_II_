#include "mesh.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

#include "common/debug.h"

namespace s21 {

const std::vector<Point>& Mesh::GetVertices() const { return vertices_; }
const std::vector<Edge>& Mesh::GetEdges() const { return edges_; }

void Mesh::SetVertices(const std::vector<Point>& vertices) {
  vertices_ = vertices;
}

void Mesh::SetEdges(const std::vector<Edge>& edges) { edges_ = edges; }

void Mesh::SetNormals(const std::vector<Point>& normals) { normals_ = normals; }

void Mesh::SetUVs(const std::vector<Point2D>& uvs) { uvs_ = uvs; }

void Mesh::SetFlatNormals(const std::vector<Point>& flat_normals) {
  flat_normals_ = flat_normals;
  DEBUG_PRINT("Mesh::SetFlatNormals: size = " << flat_normals_.size());
}

void Mesh::SetSmoothNormals(const std::vector<Point>& smooth_normals) {
  smooth_normals_ = smooth_normals;
  DEBUG_PRINT("Mesh::SetSmoothNormals: size = " << smooth_normals_.size());
}

const std::vector<Point>& Mesh::GetNormals() const { return normals_; }

const std::vector<Point2D>& Mesh::GetUVs() const { return uvs_; }

const std::vector<Point>& Mesh::GetFlatNormals() const { return flat_normals_; }

const std::vector<Point>& Mesh::GetSmoothNormals() const {
  return smooth_normals_;
}

void Mesh::AddVertex(const Point& vertex) { vertices_.push_back(vertex); }
void Mesh::AddEdge(const Edge& edge) { edges_.push_back(edge); }

size_t Mesh::VertexCount() const { return vertices_.size(); }
size_t Mesh::EdgeCount() const { return edges_.size(); }

void Mesh::Clear() {
  vertices_.clear();
  edges_.clear();
  normals_.clear();
  flat_normals_.clear();
  smooth_normals_.clear();
  uvs_.clear();
  triangles_.clear();
}

void Mesh::ComputeBoundingSphere() {
  if (vertices_.empty()) {
    bounding_sphere_ = {Point{0, 0, 0}, 0.0f};
    return;
  }
  Point center{0, 0, 0};
  for (const auto& v : vertices_) {
    center.x += v.x;
    center.y += v.y;
    center.z += v.z;
  }
  center.x /= static_cast<float>(vertices_.size());
  center.y /= static_cast<float>(vertices_.size());
  center.z /= static_cast<float>(vertices_.size());

  float max_dist_sq = 0.0f;
  for (const auto& v : vertices_) {
    float dx = v.x - center.x;
    float dy = v.y - center.y;
    float dz = v.z - center.z;
    float dist_sq = dx * dx + dy * dy + dz * dz;
    if (dist_sq > max_dist_sq) max_dist_sq = dist_sq;
  }
  bounding_sphere_.center = center;
  bounding_sphere_.radius = std::sqrt(max_dist_sq);
}

BoundingBox Mesh::GetBoundingBox() const {
  if (vertices_.empty()) {
    return {Point{0, 0, 0}, Point{0, 0, 0}};
  }
  Point min = vertices_[0];
  Point max = vertices_[0];
  for (const auto& v : vertices_) {
    if (v.x < min.x) min.x = v.x;
    if (v.y < min.y) min.y = v.y;
    if (v.z < min.z) min.z = v.z;
    if (v.x > max.x) max.x = v.x;
    if (v.y > max.y) max.y = v.y;
    if (v.z > max.z) max.z = v.z;
  }
  return {min, max};
}

void Mesh::SetTriangles(const std::vector<unsigned int>& triangles) {
  triangles_ = triangles;
}

const std::vector<unsigned int>& Mesh::GetTriangles() const {
  return triangles_;
}

size_t Mesh::TriangleCount() const { return triangles_.size() / 3; }

void Mesh::ComputeNormals(float smoothingFactor) {
  DEBUG_PRINT("Mesh::ComputeNormals: factor = " << smoothingFactor);
  if (flat_normals_.empty() || smooth_normals_.empty()) {
    DEBUG_PRINT(
        "ComputeNormals: flat or smooth normals missing, cannot compute");
    return;
  }
  if (flat_normals_.size() != vertices_.size() ||
      smooth_normals_.size() != vertices_.size()) {
    DEBUG_PRINT("ComputeNormals: size mismatch");
    return;
  }

  normals_.resize(vertices_.size());
  float t = std::clamp(smoothingFactor, 0.0f, 1.0f);
  for (size_t i = 0; i < vertices_.size(); ++i) {
    const Point& flat = flat_normals_[i];
    const Point& smooth = smooth_normals_[i];
    Point blended;
    blended.x = flat.x * (1.0f - t) + smooth.x * t;
    blended.y = flat.y * (1.0f - t) + smooth.y * t;
    blended.z = flat.z * (1.0f - t) + smooth.z * t;
    float len = std::sqrt(blended.x * blended.x + blended.y * blended.y +
                          blended.z * blended.z);
    if (len > 1e-6f) {
      blended.x /= len;
      blended.y /= len;
      blended.z /= len;
    }
    normals_[i] = blended;
  }

  DEBUG_PRINT("ComputeNormals finished. First normal: ("
              << normals_[0].x << ", " << normals_[0].y << ", " << normals_[0].z
              << ")");
}

// Вспомогательная структура для оптимизации флипами
struct EdgeKey {
  unsigned int v1, v2;
  bool operator==(const EdgeKey& other) const {
    return (v1 == other.v1 && v2 == other.v2) ||
           (v1 == other.v2 && v2 == other.v1);
  }
};
struct EdgeKeyHash {
  size_t operator()(const EdgeKey& e) const {
    if (e.v1 < e.v2)
      return (static_cast<size_t>(e.v1) << 32) | e.v2;
    else
      return (static_cast<size_t>(e.v2) << 32) | e.v1;
  }
};

// Локальный хеш для пары unsigned int
struct MeshPairHash {
  std::size_t operator()(const std::pair<unsigned int, unsigned int>& p) const {
    return (static_cast<size_t>(p.first) << 32) | p.second;
  }
};

void Mesh::OptimizeTriangulation(int max_iterations) {
  if (triangles_.size() < 6) return;

  for (int iter = 0; iter < max_iterations; ++iter) {
    std::unordered_map<EdgeKey, std::pair<int, int>, EdgeKeyHash>
        edgeToTriangles;
    for (size_t i = 0; i < triangles_.size(); i += 3) {
      unsigned int i1 = triangles_[i];
      unsigned int i2 = triangles_[i + 1];
      unsigned int i3 = triangles_[i + 2];
      auto addEdge = [&](unsigned int a, unsigned int b, int triIdx) {
        EdgeKey key{a, b};
        auto it = edgeToTriangles.find(key);
        if (it == edgeToTriangles.end()) {
          edgeToTriangles[key] = {triIdx, -1};
        } else {
          it->second.second = triIdx;
        }
      };
      int triIdx = static_cast<int>(i);
      addEdge(i1, i2, triIdx);
      addEdge(i2, i3, triIdx);
      addEdge(i3, i1, triIdx);
    }

    bool flipped = false;
    for (const auto& entry : edgeToTriangles) {
      const EdgeKey& edge = entry.first;
      int tri1Idx = entry.second.first;
      int tri2Idx = entry.second.second;
      if (tri2Idx == -1) continue;

      unsigned int a1 = triangles_[tri1Idx];
      unsigned int b1 = triangles_[tri1Idx + 1];
      unsigned int c1 = triangles_[tri1Idx + 2];
      unsigned int a2 = triangles_[tri2Idx];
      unsigned int b2 = triangles_[tri2Idx + 1];
      unsigned int c2 = triangles_[tri2Idx + 2];

      unsigned int v1 = edge.v1;
      unsigned int v2 = edge.v2;

      auto findOpposite = [&](unsigned int t1, unsigned int t2, unsigned int t3,
                              unsigned int e1,
                              unsigned int e2) -> unsigned int {
        if (t1 != e1 && t1 != e2) return t1;
        if (t2 != e1 && t2 != e2) return t2;
        return t3;
      };
      unsigned int opp1 = findOpposite(a1, b1, c1, v1, v2);
      unsigned int opp2 = findOpposite(a2, b2, c2, v1, v2);

      const Point& p1 = vertices_[v1];
      const Point& p2 = vertices_[v2];
      const Point& p3 = vertices_[opp1];
      const Point& p4 = vertices_[opp2];

      auto angle = [](const Point& a, const Point& b, const Point& c) {
        Point ba = {a.x - b.x, a.y - b.y, a.z - b.z};
        Point bc = {c.x - b.x, c.y - b.y, c.z - b.z};
        float dot = ba.x * bc.x + ba.y * bc.y + ba.z * bc.z;
        float len1 = std::sqrt(ba.x * ba.x + ba.y * ba.y + ba.z * ba.z);
        float len2 = std::sqrt(bc.x * bc.x + bc.y * bc.y + bc.z * bc.z);
        if (len1 < 1e-6f || len2 < 1e-6f) return 0.0f;
        float cos_angle = dot / (len1 * len2);
        cos_angle = std::clamp(cos_angle, -1.0f, 1.0f);
        return std::acos(cos_angle);
      };

      float alpha = angle(p1, p2, p3);
      float beta = angle(p1, p2, p4);
      if (alpha + beta > 3.14159f * 0.99f) {
        triangles_[tri1Idx] = opp1;
        triangles_[tri1Idx + 1] = v1;
        triangles_[tri1Idx + 2] = opp2;
        triangles_[tri2Idx] = opp1;
        triangles_[tri2Idx + 1] = opp2;
        triangles_[tri2Idx + 2] = v2;
        flipped = true;
        break;
      }
    }
    if (!flipped) break;
  }

  std::unordered_set<std::pair<unsigned int, unsigned int>, MeshPairHash>
      edgeSet;
  for (size_t i = 0; i < triangles_.size(); i += 3) {
    unsigned int i1 = triangles_[i];
    unsigned int i2 = triangles_[i + 1];
    unsigned int i3 = triangles_[i + 2];
    auto addEdge = [&](unsigned int a, unsigned int b) {
      if (a > b) std::swap(a, b);
      edgeSet.emplace(a, b);
    };
    addEdge(i1, i2);
    addEdge(i2, i3);
    addEdge(i3, i1);
  }
  edges_.clear();
  edges_.reserve(edgeSet.size());
  for (const auto& p : edgeSet) {
    edges_.emplace_back(p.first, p.second);
  }
}

}  // namespace s21
