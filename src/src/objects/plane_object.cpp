#include "plane_object.h"
#include <cmath>
#include "scene/mesh.h"

namespace s21 {

PlaneObject::PlaneObject(const Point& point, const Point& normal)
    : point_(point), normal_(normal) {
    // нормируем нормаль
    float len = std::sqrt(normal_.x * normal_.x + normal_.y * normal_.y + normal_.z * normal_.z);
    if (len > 1e-6f) {
        normal_.x /= len; normal_.y /= len; normal_.z /= len;
    }
    SetName("Plane");
}

bool PlaneObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    float denom = normal_.x * ray.direction.x + normal_.y * ray.direction.y + normal_.z * ray.direction.z;
    if (std::abs(denom) < 1e-6f) return false;

    float t = ((point_.x - ray.origin.x) * normal_.x +
               (point_.y - ray.origin.y) * normal_.y +
               (point_.z - ray.origin.z) * normal_.z) / denom;
    if (t < t_min || t > t_max) return false;

    rec.t = t;
    rec.point = ray.origin + Point(ray.direction.x * t, ray.direction.y * t, ray.direction.z * t);
    rec.normal = normal_;
    rec.material = material_;
    return true;
}

std::unique_ptr<Mesh> PlaneObject::GenerateMesh(int /*precision*/) const {
    auto mesh = std::make_unique<Mesh>();
    // Генерация большого квадрата, перпендикулярного нормали
    // Для простоты создадим квадрат 10x10 в плоскости XZ, если нормаль (0,1,0)
    if (std::abs(normal_.y - 1.0f) < 1e-3f) {
        float y = point_.y;
        mesh->AddVertex({-5, y, -5});
        mesh->AddVertex({ 5, y, -5});
        mesh->AddVertex({ 5, y,  5});
        mesh->AddVertex({-5, y,  5});
        mesh->AddEdge({0, 1});
        mesh->AddEdge({1, 2});
        mesh->AddEdge({2, 3});
        mesh->AddEdge({3, 0});
    }
    // TODO: другие ориентации
    return mesh;
}

} // namespace s21
