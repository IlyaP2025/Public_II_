#include "plane_object.h"
#include <cmath>
#include "scene/mesh.h"

#include "plane_object.h"
#include <cmath>
#include "scene/mesh.h"

namespace s21 {

PlaneObject::PlaneObject(const Point& point, const Point& normal) : point_(point), normal_(normal) {
    float len = std::sqrt(normal_.x * normal_.x + normal_.y * normal_.y + normal_.z * normal_.z);
    if (len > 1e-6f) {
        normal_.x /= len; normal_.y /= len; normal_.z /= len;
    }
    SetName("Floor");
}

PlaneObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
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
    if (std::abs(normal_.y - 1.0f) < 1e-3f) {
        float y = point_.y;
        std::vector<Point> vertices = {
            {-5, y, -5}, {5, y, -5}, {5, y, 5}, {-5, y, 5}
        };
        mesh->SetVertices(vertices);
        mesh->SetEdges({
            Edge{0, 1}, Edge{1, 2}, Edge{2, 3}, Edge{3, 0}
        });
        mesh->SetNormals(std::vector<Point>(4, {0, 1, 0}));
        mesh->SetUVs(std::vector<Point2D>(4, {0, 0}));
        mesh->SetTriangles({0, 1, 2, 0, 2, 3}); // Два треугольника
    }
    mesh->ComputeBoundingSphere();
    return mesh;
}

} // namespace s21
