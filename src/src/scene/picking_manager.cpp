#include "common/transform.h"
#include "picking_manager.h"
#include <cmath>
#include <limits>
#include "common/point.h"
#include "matrix/s21_matrix_oop.h"
#include "scene/mesh.h"

namespace s21 {

namespace {

Point Normalize(const Point& v) {
  float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len < 1e-6f) return Point{0, 0, 0};
  return Point{v.x / len, v.y / len, v.z / len};
}

float Dot(const Point& a, const Point& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

std::optional<float> IntersectRaySphere(const Ray& ray,
                                        const Point& center,
                                        float radius,
                                        Point& hit_point) {
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

  hit_point = ray.origin + Point(ray.direction.x * t,
                                 ray.direction.y * t,
                                 ray.direction.z * t);
  return t;
}

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

  Point nearWorld = TransformPoint(invView,
      Point(static_cast<float>(nearEye(0, 0)),
            static_cast<float>(nearEye(1, 0)),
            static_cast<float>(nearEye(2, 0))));
  Point farWorld = TransformPoint(invView,
      Point(static_cast<float>(farEye(0, 0)),
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

}  // namespace

Mesh* PickingManager::PickObject(int mouse_x, int mouse_y,
                                 int width, int height,
                                 const Camera& camera, const Scene& scene) {
  Ray worldRay = ComputePickRay(mouse_x, mouse_y, width, height, camera);

  if (scene.IsStructureDirty())
    const_cast<Scene&>(scene).RebuildSpatialIndex();
  auto* index = const_cast<Scene&>(scene).GetSpatialIndex();

  Mesh* selected = nullptr;
  float closest_t = std::numeric_limits<float>::max();

  auto checkMesh = [&](Mesh* mesh) {
    if (!mesh) return;
    S21Matrix model = mesh->GetTransform().GetModelMatrix();
    S21Matrix invModel;
    try {
      double det = model.Determinant();
      if (std::abs(det) < 1e-7) return;
      invModel = model.InverseMatrix();
    } catch (const std::exception&) {
      return;
    }

    Ray localRay;
    localRay.origin = TransformPoint(invModel, worldRay.origin);
    Point worldEnd = worldRay.origin + worldRay.direction;
    Point localEnd = TransformPoint(invModel, worldEnd);
    localRay.direction = Normalize(localEnd - localRay.origin);

    const BoundingSphere& sphere = mesh->GetBoundingSphere();
    Point hit_point;
    auto t_opt = IntersectRaySphere(localRay, sphere.center,
                                    sphere.radius, hit_point);

    if (t_opt.has_value() && *t_opt < closest_t) {
      closest_t = *t_opt;
      selected = mesh;
    }
  };

  if (index) {
    std::vector<size_t> candidates =
        index->QueryRay(worldRay.origin, worldRay.direction);
    const auto& objects = scene.GetObjects();
    for (size_t idx : candidates) {
      if (idx >= objects.size()) continue;
      Mesh* mesh = dynamic_cast<Mesh*>(objects[idx].get());
      checkMesh(mesh);
    }
  } else {
    for (const auto& obj : scene.GetObjects()) {
      Mesh* mesh = dynamic_cast<Mesh*>(obj.get());
      checkMesh(mesh);
    }
  }

  return selected;
}

}  // namespace s21
