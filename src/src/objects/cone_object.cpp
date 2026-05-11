#include "cone_object.h"
#include <cmath>
#include <vector>
#include "common/point.h"
#include "scene/mesh.h"
#include "common/debug.h"

namespace s21 {

ConeObject::ConeObject(float radius, float height, const Point& center)
    : radius_(radius), height_(height), center_(center) {
    SetName("Cone");
}

bool ConeObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Point apex = {center_.x, center_.y + height_ / 2.0f, center_.z};
    Point baseCenter = {center_.x, center_.y - height_ / 2.0f, center_.z};
    float k = (radius_ / height_) * (radius_ / height_);

    Point oc = ray.origin - apex;
    float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z - k * ray.direction.y * ray.direction.y;
    float b = 2.0f * (oc.x * ray.direction.x + oc.z * ray.direction.z - k * oc.y * ray.direction.y);
    float c = oc.x * oc.x + oc.z * oc.z - k * oc.y * oc.y;

    float disc = b * b - 4 * a * c;
    if (disc < 0) return false;

    float sqrtDisc = std::sqrt(disc);
    float t = (-b - sqrtDisc) / (2.0f * a);
    if (t < t_min || t > t_max) {
        t = (-b + sqrtDisc) / (2.0f * a);
        if (t < t_min || t > t_max) return false;
    }

    Point p = ray.origin + Point(ray.direction.x * t, ray.direction.y * t, ray.direction.z * t);
    if (p.y < baseCenter.y || p.y > apex.y) return false;

    rec.t = t;
    rec.point = p;
    Point axisPoint = {center_.x, p.y, center_.z};
    Point normal = p - axisPoint;
    float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (len > 1e-6f) {
        rec.normal = Point(normal.x / len, normal.y / len, normal.z / len);
    } else {
        rec.normal = Point(0, -1, 0);
    }
    rec.material = material_;
    return true;
}

std::unique_ptr<Mesh> ConeObject::GenerateMesh(int precision) const {
    auto mesh = std::make_unique<Mesh>();
    if (precision < 3) precision = 3;

    std::vector<Point> vertices;
    std::vector<unsigned int> indices;
    std::vector<Point> normals;

    float halfH = height_ / 2.0f;
    Point apex = {center_.x, center_.y + halfH, center_.z};
    Point baseCenter = {center_.x, center_.y - halfH, center_.z};

    unsigned int apexIndex = 0;
    vertices.push_back(apex);
    normals.push_back(Point(0, 1, 0));

    for (int i = 0; i <= precision; ++i) {
        float angle = 2.0f * M_PI * i / precision;
        float x = radius_ * std::cos(angle);
        float z = radius_ * std::sin(angle);
        vertices.push_back({center_.x + x, baseCenter.y, center_.z + z});
        Point n = Point(x, 0, z);
        float ln = std::sqrt(n.x * n.x + n.z * n.z);
        if (ln > 1e-6f) {
            n.x /= ln; n.z /= ln;
        }
        normals.push_back(n);
    }

    for (unsigned int i = 0; i < static_cast<unsigned int>(precision); ++i) {
        indices.push_back(i + 2);
        indices.push_back(i + 1);
        indices.push_back(apexIndex);
    }

    mesh->SetVertices(vertices);
    mesh->SetNormals(normals);
    mesh->SetTriangles(indices);
    mesh->ComputeBoundingSphere();

    std::vector<Edge> edges;
    for (size_t i = 0; i < indices.size(); i += 3) {
        edges.push_back(Edge{indices[i], indices[i+1]});
        edges.push_back(Edge{indices[i+1], indices[i+2]});
        edges.push_back(Edge{indices[i+2], indices[i]});
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
    const auto& norms = mesh->GetNormals();
    DEBUG_PRINT("  Normals: " << norms.size());
    if (!norms.empty()) {
        DEBUG_PRINT("  First normal: (" << norms[0].x << ", " << norms[0].y << ", " << norms[0].z << ")");
        if (norms.size() >= 4) {
            DEBUG_PRINT("  Last normal:  (" << norms.back().x << ", " << norms.back().y << ", " << norms.back().z << ")");
        }
    }
    const auto& tris = mesh->GetTriangles();
    DEBUG_PRINT("  Triangles: " << tris.size() / 3);
    if (tris.size() >= 6) {
        DEBUG_PRINT("  First triangle: " << tris[0] << ", " << tris[1] << ", " << tris[2]);
        if (mesh->GetVertices().size() > tris[2]) {
            const Point& a = mesh->GetVertices()[tris[0]];
            const Point& b = mesh->GetVertices()[tris[1]];
            const Point& c = mesh->GetVertices()[tris[2]];
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
