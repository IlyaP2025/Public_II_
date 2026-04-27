#ifndef S21_OBJ_LOADER_H
#define S21_OBJ_LOADER_H

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "common/model_data.h"
#include "iloader.h"
#include "scene/mesh.h"

namespace s21 {

struct PairHash {
  std::size_t operator()(const std::pair<unsigned int, unsigned int>& p) const {
    return (static_cast<size_t>(p.first) << 32) | p.second;
  }
};

class ObjLoader : public ILoader {
 public:
  bool Load(const std::string& filename,
            std::vector<std::unique_ptr<SceneObject>>& out_objects,
            std::string& error) override;

  static RawModelData LoadData(const std::string& filename,
                               float smoothingFactor = 1.0f);
  static int ResolveIndex(int index, size_t vertex_count);

 private:
  struct RawData {
    std::vector<Point> vertices;
    std::vector<Point2D> uvs;

    std::vector<Point> expanded_vertices;
    std::vector<Point> expanded_normals;
    std::vector<Point> flat_normals;
    std::vector<Point> smooth_normals;
    std::vector<Point2D> expanded_uvs;
    std::vector<int> expanded_original_indices;
    std::vector<Edge> expanded_edges;
    std::vector<unsigned int> expanded_triangles;
  };

  static bool ParseFile(const std::string& filename, RawData& data,
                        std::string& error);
  static void AddExpandedTriangle(
      RawData& data, int v1, int v2, int v3, int uv1, int uv2, int uv3,
      std::unordered_set<std::pair<unsigned int, unsigned int>, PairHash>&
          edgeSet);
  static void ComputeSmoothNormals(RawData& data, float smoothingFactor);
};

}  // namespace s21

#endif
