#include "sphere_object.h"
#include <cmath>
#include "common/point.h"
#include "scene/mesh.h"

namespace s21 {

SphereObject::SphereObject(float radius, const Point& center)
    : radius_(radius), center_(center) {
    SetName("Sphere");
}

bool SphereObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Point oc = ray.origin - center_;
    float a = ray.direction.x * ray.direction.x +
              ray.direction.y * ray.direction.y +
              ray.direction.z * ray.direction.z;
    float b = 2.0f * (oc.x * ray.direction.x + oc.y * ray.direction.y + oc.z * ray.direction.z);
    float c = (oc.x * oc.x + oc.y * oc.y + oc.z * oc.z) - radius_ * radius_;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float sqrtd = std::sqrt(discriminant);
    float root = (-b - sqrtd) / (2.0f * a);
    if (root < t_min || root > t_max) {
        root = (-b + sqrtd) / (2.0f * a);
        if (root < t_min || root > t_max) return false;
    }

    rec.t = root;
    rec.point = ray.origin + Point(ray.direction.x * root,
                                  ray.direction.y * root,
                                  ray.direction.z * root);
    rec.normal = (rec.point - center_) * (1.0f / radius_);
    rec.material = material_;
    return true;
}

std::unique_ptr<Mesh> SphereObject::GenerateMesh(int precision) const {
    auto mesh = std::make_unique<Mesh>();
    // Генерация вершин и индексов сферы (можно взять из старого Sphere класса, если есть)
    // Здесь оставлю заготовку - вы можете скопировать код из процедурной генерации сферы
    return mesh;
}

} // namespace s21
