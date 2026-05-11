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

    // 24 вершины (как в OBJ), по 4 на каждую грань
    std::vector<Point> verts = {
        // bottom (y-)
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x - h, center_.y - h, center_.z + h},
        // top (y+)
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
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
    };

    // Исправленные индексы (правая грань теперь смотрит наружу)
    std::vector<unsigned int> tris = {
        // bottom
        0,1,2,  0,2,3,
        // top
        4,7,6,  4,6,5,
        // front
        8,11,10, 8,10,9,
        // back
        12,13,14, 12,14,15,
        // left
        16,17,18, 16,18,19,
        // right (исправлено: 20,22,21 и 20,23,22)
        20,22,21, 20,23,22
    };

    mesh->SetTriangles(tris);

    std::vector<Point> norms = {
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, // bottom
        {0,  1, 0}, {0,  1, 0}, {0,  1, 0}, {0,  1, 0}, // top
        {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, // front
        {0, 0,  1}, {0, 0,  1}, {0, 0,  1}, {0, 0,  1}, // back
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, // left
        {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}      // right
    };
    mesh->SetNormals(norms);

    mesh->SetVertices(verts);
    mesh->ComputeBoundingSphere();

    std::vector<Edge> edges;
    for (size_t i = 0; i < tris.size(); i += 3) {
        edges.push_back(Edge{tris[i], tris[i+1]});
        edges.push_back(Edge{tris[i+1], tris[i+2]});
        edges.push_back(Edge{tris[i+2], tris[i]});
    }
    mesh->SetEdges(edges);

    // Отладка
    DEBUG_PRINT("=== Mesh generated: " << GetName().c_str() << " ===");
    DEBUG_PRINT("  Vertices: " << mesh->GetVertices().size());
    const auto& v = mesh->GetVertices();
    for (size_t i = 0; i < v.size(); ++i) {
        DEBUG_PRINT("   v[" << i << "]: (" << v[i].x << ", " << v[i].y << ", " << v[i].z << ")");
    }
    const auto& n = mesh->GetNormals();
    DEBUG_PRINT("  Normals: " << n.size());
    for (size_t i = 0; i < n.size(); ++i) {
        DEBUG_PRINT("   n[" << i << "]: (" << n[i].x << ", " << n[i].y << ", " << n[i].z << ")");
    }
    const auto& t = mesh->GetTriangles();
    DEBUG_PRINT("  Triangles: " << t.size() / 3);
    for (size_t i = 0; i < t.size(); i += 3) {
        DEBUG_PRINT("   tri[" << i/3 << "]: " << t[i] << ", " << t[i+1] << ", " << t[i+2]);
        const Point& a = v[t[i]];
        const Point& b = v[t[i+1]];
        const Point& c = v[t[i+2]];
        float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        float nx = uy*vz - uz*vy;
        float ny = uz*vx - ux*vz;
        float nz = ux*vy - uy*vx;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
        DEBUG_PRINT("    computed normal: (" << nx << ", " << ny << ", " << nz << ")");
    }
    DEBUG_PRINT("=== End Mesh ===");

    return mesh;
}

} // namespace s21
