#include "plane_object.h"
#include <cmath>
#include "scene/mesh.h"

namespace s21 {

PlaneObject::PlaneObject(const Point& point, const Point& normal)
    : point_(point), normal_(normal) {
    float len = std::sqrt(normal_.x * normal_.x + normal_.y * normal_.y + normal_.z * normal_.z);
    if (len > 1e-6f) {
        normal_.x /= len; normal_.y /= len; normal_.z /= len;
    }
    SetName("Floor");
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
    // Для пола с нормалью (0,1,0) создадим два треугольника с развёрнутыми вершинами
    if (std::abs(normal_.y - 1.0f) < 1e-3f) {
        float y = point_.y;
        // Вершины квадрата
        Point v0{-5, y, -5};
        Point v1{ 5, y, -5};
        Point v2{ 5, y,  5};
        Point v3{-5, y,  5};

        // Первый треугольник: v0, v1, v2
        // Второй треугольник: v0, v2, v3
        std::vector<Point> verts = {
            v0, v1, v2,  // треугольник 1
            v0, v2, v3   // треугольник 2
        };
        std::vector<Point> norms(6, {0.0f, 1.0f, 0.0f});
        std::vector<unsigned int> tris = {0,1,2, 3,4,5};

        mesh->SetVertices(verts);
        mesh->SetNormals(norms);
        mesh->SetTriangles(tris);
        mesh->SetFlatNormals(norms);
        mesh->SetSmoothNormals(norms);
    }
    // Остальные ориентации можно добавить при необходимости
    mesh->ComputeBoundingSphere();

    std::vector<Edge> edges;
    const auto& tri = mesh->GetTriangles();
    for (size_t i = 0; i < tri.size(); i += 3) {
        edges.push_back(Edge{tri[i], tri[i+1]});
        edges.push_back(Edge{tri[i+1], tri[i+2]});
        edges.push_back(Edge{tri[i+2], tri[i]});
    }
    mesh->SetEdges(edges);
    return mesh;
}

} // namespace s21
