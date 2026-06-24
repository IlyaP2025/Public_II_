#include "pyramid_object.h"
#include "scene/mesh.h"
#include "common/debug.h"
#include <cmath>
#include <limits>

namespace s21 {

// Вспомогательная функция пересечения луча с треугольником
static bool IntersectTriangle(const Ray& ray,
                              const Point& v0, const Point& v1, const Point& v2,
                              float t_min, float t_max,
                              float& t_out, Point& normal_out) {
    const float EPS = 1e-6f;
    Point e1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
    Point e2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
    Point h = {
        ray.direction.y * e2.z - ray.direction.z * e2.y,
        ray.direction.z * e2.x - ray.direction.x * e2.z,
        ray.direction.x * e2.y - ray.direction.y * e2.x
    };
    float a = e1.x * h.x + e1.y * h.y + e1.z * h.z;
    if (std::abs(a) < EPS) return false;

    float f = 1.0f / a;
    Point s = {ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z};
    float u = f * (s.x * h.x + s.y * h.y + s.z * h.z);
    if (u < 0.0f || u > 1.0f) return false;

    Point q = {
        s.y * e1.z - s.z * e1.y,
        s.z * e1.x - s.x * e1.z,
        s.x * e1.y - s.y * e1.x
    };
    float v = f * (ray.direction.x * q.x + ray.direction.y * q.y + ray.direction.z * q.z);
    if (v < 0.0f || u + v > 1.0f) return false;

    float t = f * (e2.x * q.x + e2.y * q.y + e2.z * q.z);
    if (t < t_min || t > t_max) return false;

    t_out = t;
    normal_out = {
        e1.y * e2.z - e1.z * e2.y,
        e1.z * e2.x - e1.x * e2.z,
        e1.x * e2.y - e1.y * e2.x
    };
    float len = std::sqrt(normal_out.x * normal_out.x + normal_out.y * normal_out.y + normal_out.z * normal_out.z);
    if (len > EPS) {
        normal_out.x /= len;
        normal_out.y /= len;
        normal_out.z /= len;
    }
    return true;
}

PyramidObject::PyramidObject(float base, float height, const Point& center)
    : base_(base), height_(height), center_(center) {
    SetName("Pyramid");
}

bool PyramidObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    float h = height_ / 2.0f;
    float b = base_ / 2.0f;
    Point apex = {center_.x, center_.y + h, center_.z};
    Point basePts[4] = {
        {center_.x - b, center_.y - h, center_.z - b},
        {center_.x + b, center_.y - h, center_.z - b},
        {center_.x + b, center_.y - h, center_.z + b},
        {center_.x - b, center_.y - h, center_.z + b}
    };

    float closest_t = t_max;
    Point closest_normal;
    bool hit = false;

    // Боковые грани
    for (int i = 0; i < 4; ++i) {
        float t;
        Point normal;
        if (IntersectTriangle(ray, apex, basePts[i], basePts[(i + 1) % 4],
                              t_min, closest_t, t, normal)) {
            if (t < closest_t) {
                closest_t = t;
                closest_normal = normal;
                hit = true;
            }
        }
    }

    // Основание (два треугольника)
    {
        float t;
        Point normal;
        if (IntersectTriangle(ray, basePts[0], basePts[1], basePts[2],
                              t_min, closest_t, t, normal)) {
            if (t < closest_t) {
                closest_t = t;
                closest_normal = normal;
                hit = true;
            }
        }
        if (IntersectTriangle(ray, basePts[0], basePts[2], basePts[3],
                              t_min, closest_t, t, normal)) {
            if (t < closest_t) {
                closest_t = t;
                closest_normal = normal;
                hit = true;
            }
        }
    }

    if (!hit) return false;
    rec.t = closest_t;
    rec.point = {
        ray.origin.x + ray.direction.x * closest_t,
        ray.origin.y + ray.direction.y * closest_t,
        ray.origin.z + ray.direction.z * closest_t
    };
    rec.normal = closest_normal;
    rec.material = material_;
    return true;
}

// Оригинальная реализация GenerateMesh (взята из исходного проекта)
std::unique_ptr<Mesh> PyramidObject::GenerateMesh(int /*precision*/) const {
    auto mesh = std::make_unique<Mesh>();
    float h = height_ / 2.0f;
    float b = base_ / 2.0f;
    Point apex = {center_.x, center_.y + h, center_.z};
    Point basePts[4] = {
        {center_.x - b, center_.y - h, center_.z - b},
        {center_.x + b, center_.y - h, center_.z - b},
        {center_.x + b, center_.y - h, center_.z + b},
        {center_.x - b, center_.y - h, center_.z + b}
    };

    std::vector<Point> verts;
    std::vector<Point> norms;
    std::vector<unsigned int> tris;

    // Боковые грани
    for (int i = 0; i < 4; ++i) {
        const Point& a = basePts[i];
        const Point& bPt = basePts[(i+1)%4];
        Point u = {bPt.x - a.x, bPt.y - a.y, bPt.z - a.z};
        Point v = {apex.x - a.x, apex.y - a.y, apex.z - a.z};
        Point n = {
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        };
        float len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
        if (len > 1e-6f) { n.x /= len; n.y /= len; n.z /= len; }
        verts.push_back(apex); norms.push_back(n);
        verts.push_back(a);    norms.push_back(n);
        verts.push_back(bPt);  norms.push_back(n);
    }

    // Основание (два треугольника)
    Point nBase = {0, -1, 0};
    verts.push_back(basePts[0]); norms.push_back(nBase);
    verts.push_back(basePts[1]); norms.push_back(nBase);
    verts.push_back(basePts[2]); norms.push_back(nBase);

    verts.push_back(basePts[0]); norms.push_back(nBase);
    verts.push_back(basePts[2]); norms.push_back(nBase);
    verts.push_back(basePts[3]); norms.push_back(nBase);

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

    DEBUG_PRINT("=== Mesh generated: " << GetName().c_str() << " ===");
    DEBUG_PRINT("  Vertices: " << mesh->GetVertices().size());
    DEBUG_PRINT("  Normals: " << mesh->GetNormals().size());
    DEBUG_PRINT("  Triangles: " << mesh->GetTriangles().size() / 3);
    if (mesh->GetVertices().size() >= 3) {
        const auto& v = mesh->GetVertices();
        DEBUG_PRINT("  First vertex: (" << v[0].x << ", " << v[0].y << ", " << v[0].z << ")");
        DEBUG_PRINT("  Last vertex: (" << v.back().x << ", " << v.back().y << ", " << v.back().z << ")");
        const Point& a = v[0], &c = v[1], &d = v[2];
        float ux = c.x - a.x, uy = c.y - a.y, uz = c.z - a.z;
        float vx = d.x - a.x, vy = d.y - a.y, vz = d.z - a.z;
        float nx = uy*vz - uz*vy, ny = uz*vx - ux*vz, nz = ux*vy - uy*vx;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
        DEBUG_PRINT("  Computed normal of first triangle: (" << nx << ", " << ny << ", " << nz << ")");
    }
    DEBUG_PRINT("=== End Mesh ===");

    return mesh;
}

} // namespace s21
