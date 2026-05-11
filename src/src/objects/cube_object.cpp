#include "cube_object.h"
#include "scene/mesh.h"

namespace s21 {

CubeObject::CubeObject(float size, const Point& center)
    : size_(size), center_(center) {
    SetName("Cube");
}

bool CubeObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Point min = {center_.x - size_/2, center_.y - size_/2, center_.z - size_/2};
    Point max = {center_.x + size_/2, center_.y + size_/2, center_.z + size_/2};
    float tmin = (min.x - ray.origin.x) / ray.direction.x;
    float tmax = (max.x - ray.origin.x) / ray.direction.x;
    if (tmin > tmax) std::swap(tmin, tmax);
    float tymin = (min.y - ray.origin.y) / ray.direction.y;
    float tymax = (max.y - ray.origin.y) / ray.direction.y;
    if (tymin > tymax) std::swap(tymin, tymax);
    if ((tmin > tymax) || (tymin > tmax)) return false;
    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;
    float tzmin = (min.z - ray.origin.z) / ray.direction.z;
    float tzmax = (max.z - ray.origin.z) / ray.direction.z;
    if (tzmin > tzmax) std::swap(tzmin, tzmax);
    if ((tmin > tzmax) || (tzmin > tmax)) return false;
    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;
    float t = tmin;
    if (t < t_min) {
        t = tmax;
        if (t < t_min || t > t_max) return false;
    }
    if (t > t_max) return false;
    rec.t = t;
    rec.point = ray.origin + Point(ray.direction.x * t, ray.direction.y * t, ray.direction.z * t);
    Point c = rec.point - center_;
    float absX = std::abs(c.x);
    float absY = std::abs(c.y);
    float absZ = std::abs(c.z);
    if (absX > absY && absX > absZ) {
        rec.normal = Point(c.x > 0 ? 1.0f : -1.0f, 0, 0);
    } else if (absY > absZ) {
        rec.normal = Point(0, c.y > 0 ? 1.0f : -1.0f, 0);
    } else {
        rec.normal = Point(0, 0, c.z > 0 ? 1.0f : -1.0f);
    }
    rec.material = material_;
    return true;
}

std::unique_ptr<Mesh> CubeObject::GenerateMesh(int /*precision*/) const {
    auto mesh = std::make_unique<Mesh>();
    float h = size_ / 2.0f;
    // 24 вершины (по 4 на каждую грань)
    std::vector<Point> verts = {
        // front (z-)
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y + h, center_.z - h},
        // back (z+)
        {center_.x - h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        // left (x-)
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x - h, center_.y - h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z - h},
        // right (x+)
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z - h},
        // top (y+)
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        // bottom (y-)
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x - h, center_.y - h, center_.z + h},
    };
    mesh->SetVertices(verts);
    // 12 треугольников (по 2 на каждую грань)
    mesh->SetTriangles({
        0,1,2, 0,2,3,    // front
        4,5,6, 4,6,7,    // back
        8,9,10, 8,10,11, // left
        12,13,14, 12,14,15, // right
        16,17,18, 16,18,19, // top
        20,21,22, 20,22,23  // bottom
    });

    // Нормали для каждой грани (одинаковы для всех 4 вершин грани)
    std::vector<Point> norms = {
        {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, // front
        {0, 0,  1}, {0, 0,  1}, {0, 0,  1}, {0, 0,  1}, // back
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, // left
        {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},     // right
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},     // top
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}  // bottom
    };
    mesh->SetNormals(norms);
    mesh->ComputeBoundingSphere();

    // Рёбра
    std::vector<Edge> edges;
    const auto& tris = mesh->GetTriangles();
    for (size_t i = 0; i < tris.size(); i += 3) {
        edges.push_back(Edge{tris[i], tris[i+1]});
        edges.push_back(Edge{tris[i+1], tris[i+2]});
        edges.push_back(Edge{tris[i+2], tris[i]});
    }
    mesh->SetEdges(edges);
    return mesh;
}

} // namespace s21
