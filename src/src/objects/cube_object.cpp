#include "cube_object.h"
#include "scene/mesh.h"
#include "common/debug.h"

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
    std::vector<Point> verts = {
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x - h, center_.y - h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x - h, center_.y - h, center_.z + h},
    };
    mesh->SetVertices(verts);
    mesh->SetTriangles({
        0,2,1,   0,3,2,
        4,5,6,   4,6,7,
        8,9,10,  8,10,11,
        12,14,13, 12,15,14,
        16,18,17, 16,19,18,
        20,21,22, 20,22,23
    });
    std::vector<Point> norms = {
        {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
        {0, 0,  1}, {0, 0,  1}, {0, 0,  1}, {0, 0,  1},
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},
        {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}
    };
    mesh->SetNormals(norms);
    mesh->ComputeBoundingSphere();
    std::vector<Edge> edges;
    const auto& tris = mesh->GetTriangles();
    for (size_t i = 0; i < tris.size(); i += 3) {
        edges.push_back(Edge{tris[i], tris[i+1]});
        edges.push_back(Edge{tris[i+1], tris[i+2]});
        edges.push_back(Edge{tris[i+2], tris[i]});
    }
    mesh->SetEdges(edges);

    // Отладка
    DEBUG_PRINT("=== Mesh generated: " << GetName().c_str() << " ===");
    DEBUG_PRINT("  Vertices: " << mesh->GetVertices().size());
    if (!mesh->GetVertices().empty()) {
        const auto& v = mesh->GetVertices();
        DEBUG_PRINT("  First vertex: (" << v[0].x << ", " << v[0].y << ", " << v[0].z << ")");
        DEBUG_PRINT("  Last vertex:  (" << v.back().x << ", " << v.back().y << ", " << v.back().z << ")");
    }
    const auto& norms2 = mesh->GetNormals();
    DEBUG_PRINT("  Normals: " << norms2.size());
    if (!norms2.empty()) {
        DEBUG_PRINT("  First normal: (" << norms2[0].x << ", " << norms2[0].y << ", " << norms2[0].z << ")");
        if (norms2.size() >= 4) {
            DEBUG_PRINT("  Last normal:  (" << norms2.back().x << ", " << norms2.back().y << ", " << norms2.back().z << ")");
        }
    }
    const auto& tris2 = mesh->GetTriangles();
    DEBUG_PRINT("  Triangles: " << tris2.size() / 3);
    if (tris2.size() >= 6) {
        DEBUG_PRINT("  First triangle: " << tris2[0] << ", " << tris2[1] << ", " << tris2[2]);
        if (mesh->GetVertices().size() > tris2[2]) {
            const Point& a = mesh->GetVertices()[tris2[0]];
            const Point& b = mesh->GetVertices()[tris2[1]];
            const Point& c = mesh->GetVertices()[tris2[2]];
            float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
            float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
            float nx = uy*vz - uz*vy;
            float ny = uz*vx - ux*vz;
            float nz = ux*vy - uy*vx;
            float len = std::sqrt(nx*nx + ny*ny + nz*nz);
            if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
            DEBUG_PRINT("  Computed normal of first triangle: (" << nx << ", " << ny << ", " << nz << ")");
        }
    }
    DEBUG_PRINT("=== End Mesh ===");

    return mesh;
}

} // namespace s21
