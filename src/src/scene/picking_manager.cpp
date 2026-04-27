#include "picking_manager.h"

#include <cmath>
#include <limits>

#include "common/point.h"
#include "common/transform.h"
#include "matrix/s21_matrix_oop.h"
#include "scene/mesh.h"

namespace s21 {

namespace {

Point TransformPoint(const S21Matrix& m, const Point& p) {
  double x = m(0, 0) * p.x + m(0, 1) * p.y + m(0, 2) * p.z + m(0, 3);
  double y = m(1, 0) * p.x + m(1, 1) * p.y + m(1, 2) * p.z + m(1, 3);
  double z = m(2, 0) * p.x + m(2, 1) * p.y + m(2, 2) * p.z + m(2, 3);
  double w = m(3, 0) * p.x + m(3, 1) * p.y + m(3, 2) * p.z + m(3, 3);
  if (std::abs(w) > 1e-6) {
    return Point(static_cast<float>(x / w), static_cast<float>(y / w),
                 static_cast<float>(z / w));
  } else {
    return Point(static_cast<float>(x), static_cast<float>(y),
                 static_cast<float>(z));
  }
}

Point Normalize(const Point& v) {
  float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len < 1e-6f) return Point{0, 0, 0};
  return Point{v.x / len, v.y / len, v.z / len};
}

float Dot(const Point& a, const Point& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

std::optional<float> IntersectRaySphere(const Ray& ray, const Point& center,
                                        float radius, Point& hit_point) {
  Point oc = ray.origin - center;
  float a = Dot(ray.direction, ray.direction);
  float b = 2.0f * Dot(oc, ray.direction);
  float c = Dot(oc, oc) - radius * radius;

  float discriminant = b * b - 4 * a * c;
  const float eps = 1e-6f;
  if (discriminant < -eps) return std::nullopt;
  if (discriminant < 0) discriminant = 0.0f;

  float sqrt_disc = std::sqrt(discriminant);
  float t1 = (-b - sqrt_disc) / (2.0f * a);
  float t2 = (-b + sqrt_disc) / (2.0f * a);

  float t = std::min(t1, t2);
  if (t < 0) t = std::max(t1, t2);
  if (t < 0) return std::nullopt;

  hit_point = ray.origin + Point(ray.direction.x * t, ray.direction.y * t,
                                 ray.direction.z * t);
  return t;
}

// Вычисляет мировой луч по координатам мыши
Ray ComputePickRay(int mouse_x, int mouse_y, int width, int height,
                   const Camera& camera) {
  float ndc_x = (2.0f * mouse_x) / width - 1.0f;
  float ndc_y = 1.0f - (2.0f * mouse_y) / height;

  float aspect = static_cast<float>(width) / static_cast<float>(height);
  S21Matrix proj = camera.GetProjectionMatrix(aspect);
  S21Matrix view = camera.GetViewMatrix();
  S21Matrix invProj, invView;
  try {
    invProj = proj.InverseMatrix();
    invView = view.InverseMatrix();
  } catch (const std::exception&) {
    return {Point{0, 0, 0}, Point{0, 0, 1}};
  }

  S21Matrix nearNDC(4, 1), farNDC(4, 1);
  nearNDC(0, 0) = ndc_x;
  nearNDC(1, 0) = ndc_y;
  nearNDC(2, 0) = -1.0;
  nearNDC(3, 0) = 1.0;
  farNDC(0, 0) = ndc_x;
  farNDC(1, 0) = ndc_y;
  farNDC(2, 0) = 1.0;
  farNDC(3, 0) = 1.0;

  S21Matrix nearEye = invProj * nearNDC;
  S21Matrix farEye = invProj * farNDC;

  nearEye.MulNumber(1.0 / nearEye(3, 0));
  farEye.MulNumber(1.0 / farEye(3, 0));

  Point nearWorld =
      TransformPoint(invView, Point(static_cast<float>(nearEye(0, 0)),
                                    static_cast<float>(nearEye(1, 0)),
                                    static_cast<float>(nearEye(2, 0))));
  Point farWorld =
      TransformPoint(invView, Point(static_cast<float>(farEye(0, 0)),
                                    static_cast<float>(farEye(1, 0)),
                                    static_cast<float>(farEye(2, 0))));

  Point dir = farWorld - nearWorld;
  float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
  if (len > 1e-6f) {
    dir.x /= len;
    dir.y /= len;
    dir.z /= len;
  }

  return {nearWorld, dir};
}

Mesh* PickingManager::PickObject(int mouse_x, int mouse_y,
                                 int width, int height,
                                 const Camera& camera, const Scene& scene) {
  Ray worldRay = ComputePickRay(mouse_x, mouse_y, width, height, camera);

  if (scene.IsStructureDirty())
    const_cast<Scene&>(scene).RebuildSpatialIndex();
  auto* index = const_cast<Scene&>(scene).GetSpatialIndex();

  if (!index) {
    // fallback на полный перебор
    return PickObjectBruteForce(scene, worldRay);
  }

  std::vector<size_t> candidates = index->QueryRay(worldRay.origin, worldRay.direction);
  // дальнейшая точная проверка как в примере
}

}  // namespace s21
