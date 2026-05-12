#include "cylinder_object.h"
#include <cmath>
#include <vector>
#include "common/point.h"
#include "scene/mesh.h"
#include "common/debug.h"

namespace s21 {

CylinderObject::CylinderObject(float radius, float height, const Point& center)
    : radius_(radius), height_(height), center_(center) {
    SetName("Cylinder");
}

bool CylinderObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
    float b = 2.0f * (ray.direction.x * (ray.origin.x - center_.x) +
                     ray.direction.z * (ray.origin.z - center_.z));
    float c = (ray.origin.x - center_.x) * (ray.origin.x - center_.x) +
              (ray.origin.z - center_.z) * (ray.origin.z - center_.z) - radius_ * radius_;

    float disc = b * b - 4 * a * c;
    if (disc < 0) return false;

    float sqrtDisc = std::sqrt(disc);
    float t = (-b - sqrtDisc) / (2.0f * a);
    if (t < t_min || t > t_max) {
        t = (-b + sqrtDisc) / (2.0f * a);
        if (t < t_min || t > t_max) return false;
    }

    Point p = ray.origin + Point(ray.direction.x * t, ray.direction.y * t, ray.direction.z * t);
    float halfH = height_ / 2.0f;
    if (p.y < center_.y - halfH || p.y > center_.y + halfH) return false;

    rec.t = t;
    rec.point = p;
    float nx = 2.0f * (p.x - center_.x);
    float nz = 2.0f * (p.z - center_.z);
    float len = std::sqrt(nx * nx + nz * nz);
    if (len > 1e-6f) {
        rec.normal = Point(nx / len, 0.0f, nz / len);
    } else {
        rec.normal = Point(0, 1, 0);
    }
    rec.material = material_;
    return true;
}

std::unique_ptr<Mesh> CylinderObject::GenerateMesh(int precision) const {
    auto mesh = std::make_unique<Mesh>();
    if (precision < 3) precision = 3;

    std::vector<Point> verts;
    std::vector<Point> norms;
    std::vector<unsigned int> tris;

    float halfH = height_ / 2.0f;
    Point topCenter(center_.x, center_.y + halfH, center_.z);
    Point bottomCenter(center_.x, center_.y - halfH, center_.z);

    // ---------- Боковая поверхность (каждый треугольник отдельно) ----------
    for (int i = 0; i < precision; ++i) {
        float angle1 = 2.0f * M_PI * i / precision;
        float angle2 = 2.0f * M_PI * (i + 1) / precision;
        float x1 = radius_ * std::cos(angle1);
        float z1 = radius_ * std::sin(angle1);
        float x2 = radius_ * std::cos(angle2);
        float z2 = radius_ * std::sin(angle2);
        Point normal1(std::cos(angle1), 0.0f, std::sin(angle1));
        Point normal2(std::cos(angle2), 0.0f, std::sin(angle2));

        // Первый треугольник
        verts.push_back({center_.x + x1, topCenter.y, center_.z + z1});
        norms.push_back(normal1);
        verts.push_back({center_.x + x1, bottomCenter.y, center_.z + z1});
        norms.push_back(normal1);
        verts.push_back({center_.x + x2, topCenter.y, center_.z + z2});
        norms.push_back(normal2);

        // Второй треугольник
        verts.push_back({center_.x + x2, topCenter.y, center_.z + z2});
        norms.push_back(normal2);
        verts.push_back({center_.x + x1, bottomCenter.y, center_.z + z1});
        norms.push_back(normal1);
        verts.push_back({center_.x + x2, bottomCenter.y, center_.z + z2});
        norms.push_back(normal2);
    }

    // ---------- Верхняя крышка (нормаль строго вверх) ----------
    Point normTop(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < precision; ++i) {
        float angle1 = 2.0f * M_PI * i / precision;
        float angle2 = 2.0f * M_PI * (i + 1) / precision;
        float x1 = radius_ * std::cos(angle1);
        float z1 = radius_ * std::sin(angle1);
        float x2 = radius_ * std::cos(angle2);
        float z2 = radius_ * std::sin(angle2);

        Point v0 = topCenter;  // центр крышки
        Point v1 = {center_.x + x1, topCenter.y, center_.z + z1};
        Point v2 = {center_.x + x2, topCenter.y, center_.z + z2};

        // Треугольник: центр -> v1 -> v2 (обход против часовой стрелки сверху)
        verts.push_back(v0); norms.push_back(normTop);
        verts.push_back(v1); norms.push_back(normTop);
        verts.push_back(v2); norms.push_back(normTop);
    }

    // ---------- Нижняя крышка (нормаль строго вниз) ----------
    Point normBottom(0.0f, -1.0f, 0.0f);
    for (int i = 0; i < precision; ++i) {
        float angle1 = 2.0f * M_PI * i / precision;
        float angle2 = 2.0f * M_PI * (i + 1) / precision;
        float x1 = radius_ * std::cos(angle1);
        float z1 = radius_ * std::sin(angle1);
        float x2 = radius_ * std::cos(angle2);
        float z2 = radius_ * std::sin(angle2);

        Point v0 = bottomCenter;  // центр дна
        Point v1 = {center_.x + x1, bottomCenter.y, center_.z + z1};
        Point v2 = {center_.x + x2, bottomCenter.y, center_.z + z2};

        // Треугольник: центр -> v2 -> v1 (обход против часовой стрелки снизу)
        verts.push_back(v0); norms.push_back(normBottom);
        verts.push_back(v2); norms.push_back(normBottom);
        verts.push_back(v1); norms.push_back(normBottom);
    }

    // Индексы: просто последовательно
    for (size_t k = 0; k < verts.size(); k += 3) {
        tris.push_back(static_cast<unsigned int>(k));
        tris.push_back(static_cast<unsigned int>(k + 1));
        tris.push_back(static_cast<unsigned int>(k + 2));
    }

    mesh->SetVertices(verts);
    mesh->SetNormals(norms);
    mesh->SetTriangles(tris);
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
    DEBUG_PRINT("  Normals: " << mesh->GetNormals().size());
    DEBUG_PRINT("  Triangles: " << mesh->GetTriangles().size() / 3);
    if (mesh->GetVertices().size() >= 3) {
        const auto& v = mesh->GetVertices();
        DEBUG_PRINT("  First vertex: (" << v[0].x << ", " << v[0].y << ", " << v[0].z << ")");
        DEBUG_PRINT("  Last vertex: (" << v.back().x << ", " << v.back().y << ", " << v.back().z << ")");
        const Point& a = v[0]; const Point& b = v[1]; const Point& c = v[2];
        float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        float nx = uy*vz - uz*vy, ny = uz*vx - ux*vz, nz = ux*vy - uy*vx;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
        DEBUG_PRINT("  Computed normal of first triangle: (" << nx << ", " << ny << ", " << nz << ")");
    }
    DEBUG_PRINT("=== End Mesh ===");

    return mesh;
}

} // namespace s21
