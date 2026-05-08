#include "cube_object.h"
#include "scene/mesh.h"

namespace s21 {

CubeObject::CubeObject(float size, const Point& center)
    : size_(size), center_(center) {
    SetName("Cube");
}

bool CubeObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    // AABB куба
    Point min = {center_.x - size_/2, center_.y - size_/2, center_.z - size_/2};
    Point max = {center_.x + size_/2, center_.y + size_/2, center_.z + size_/2};
    // Алгоритм slabs
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
    // Нормаль: определяем, на какой грани точка
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
    std::vector<Point> verts = {
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h}
    };
    mesh->SetVertices(verts);
    mesh->SetTriangles({
        0,1,2, 0,2,3,  // front
        1,5,6, 1,6,2,  // right
        5,4,7, 5,7,6,  // back
        4,0,3, 4,3,7,  // left
        3,2,6, 3,6,7,  // top
        4,5,1, 4,1,0   // bottom
    });
    std::vector<Point> norms = {
        {0,0,-1}, {1,0,0}, {0,0,1}, {-1,0,0}, {0,1,0}, {0,-1,0}
    };
    // Нормали нужно задать для каждой вершины, но для каркаса упростим
    mesh->SetNormals(std::vector<Point>(8, Point(0,0,1)));
    mesh->ComputeBoundingSphere();
    // Рёбра
    std::vector<Edge> edges;
    const std::vector<unsigned int>& tri = mesh->GetTriangles();
    for (size_t i = 0; i < tri.size(); i += 3) {
        edges.push_back(Edge{tri[i], tri[i+1]});
        edges.push_back(Edge{tri[i+1], tri[i+2]});
        edges.push_back(Edge{tri[i+2], tri[i]});
    }
    mesh->SetEdges(edges);
    return mesh;
}

} // namespace s21
