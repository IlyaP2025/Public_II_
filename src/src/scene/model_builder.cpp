#include "model_builder.h"

#include "common/debug.h"

namespace s21 {

std::unique_ptr<Mesh> ModelBuilder::BuildFromRawData(const RawModelData& data) {
  if (!data.success) {
    throw std::runtime_error("RawModelData is not successful: " +
                             data.errorMsg);
  }

  auto mesh = std::make_unique<Mesh>();
  mesh->SetVertices(data.vertices);
  mesh->SetEdges(data.edges);
  mesh->SetNormals(data.normals);
  mesh->SetFlatNormals(data.flat_normals);
  mesh->SetSmoothNormals(data.smooth_normals);
  mesh->SetUVs(data.uvs);
  mesh->SetTriangles(data.triangles);
  mesh->ComputeBoundingSphere();
  mesh->SetSourceFile(data.filename);

  // Если UV отсутствуют, генерируем планарную проекцию (XY)
  if (mesh->GetUVs().empty()) {
    std::vector<Point2D> generatedUVs;
    const auto& verts = mesh->GetVertices();
    generatedUVs.reserve(verts.size());
    Point min(1e30f, 1e30f, 1e30f);
    Point max(-1e30f, -1e30f, -1e30f);
    for (const auto& v : verts) {
      if (v.x < min.x) min.x = v.x;
      if (v.y < min.y) min.y = v.y;
      if (v.z < min.z) min.z = v.z;
      if (v.x > max.x) max.x = v.x;
      if (v.y > max.y) max.y = v.y;
      if (v.z > max.z) max.z = v.z;
    }
    float rangeX = max.x - min.x;
    float rangeY = max.y - min.y;
    if (rangeX < 1e-6f) rangeX = 1.0f;
    if (rangeY < 1e-6f) rangeY = 1.0f;
    for (const auto& v : verts) {
      float u = (v.x - min.x) / rangeX;
      float v_val = (v.y - min.y) / rangeY;
      generatedUVs.emplace_back(u, v_val);
    }
    mesh->SetUVs(generatedUVs);
  }

  // Вычисляем нормали с учётом коэффициента сглаживания
  mesh->ComputeNormals(data.smoothingFactor);

  // Оптимизируем триангуляцию флипами Делоне
  mesh->OptimizeTriangulation(5);

  return mesh;
}

}  // namespace s21
