#include "ray_mesh_intersect.h"

#include <cmath>
#include <limits>

#include "common/ray.h"
#include "scene/mesh.h"
#include "tracer/ray_tracer.h"   // HitRecord, материал и пр.

namespace s21 {

bool RayMeshIntersect(const Mesh& mesh, const Ray& ray,
                      float t_min, float t_max, HitRecord& rec) {
    const auto& vertices = mesh.GetVertices();
    const auto& triangles = mesh.GetTriangles();
    if (triangles.empty() || vertices.empty()) return false;

    BoundingBox bbox = mesh.GetBoundingBox();
    if (!bbox.IntersectsRay(ray.origin, ray.direction))
        return false;

    bool hit = false;
    float closest_t = t_max;

    for (size_t i = 0; i < triangles.size(); i += 3) {
        unsigned i0 = triangles[i], i1 = triangles[i+1], i2 = triangles[i+2];
        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size())
            continue;

        const Point& v0 = vertices[i0], &v1 = vertices[i1], &v2 = vertices[i2];
        const float EPS = 1e-6f;
        Point e1 = {v1.x - v0.x, v1.y - v0.y, v1.z - v0.z};
        Point e2 = {v2.x - v0.x, v2.y - v0.y, v2.z - v0.z};
        Point h = {ray.direction.y*e2.z - ray.direction.z*e2.y,
                   ray.direction.z*e2.x - ray.direction.x*e2.z,
                   ray.direction.x*e2.y - ray.direction.y*e2.x};
        float a = e1.x*h.x + e1.y*h.y + e1.z*h.z;
        if (std::fabs(a) < EPS) continue;

        float f = 1.0f / a;
        Point s = {ray.origin.x - v0.x, ray.origin.y - v0.y, ray.origin.z - v0.z};
        float u = f * (s.x*h.x + s.y*h.y + s.z*h.z);
        if (u < 0.0f || u > 1.0f) continue;

        Point q = {s.y*e1.z - s.z*e1.y, s.z*e1.x - s.x*e1.z, s.x*e1.y - s.y*e1.x};
        float v = f * (ray.direction.x*q.x + ray.direction.y*q.y + ray.direction.z*q.z);
        if (v < 0.0f || u + v > 1.0f) continue;

        float t = f * (e2.x*q.x + e2.y*q.y + e2.z*q.z);
        if (t < t_min || t > closest_t) continue;

        closest_t = t;
        rec.t = t;
        rec.point = {ray.origin.x + ray.direction.x*t,
                     ray.origin.y + ray.direction.y*t,
                     ray.origin.z + ray.direction.z*t};
        Point normal = {e1.y*e2.z - e1.z*e2.y,
                        e1.z*e2.x - e1.x*e2.z,
                        e1.x*e2.y - e1.y*e2.x};
        float len = std::sqrt(normal.x*normal.x + normal.y*normal.y + normal.z*normal.z);
        if (len > EPS) { normal.x /= len; normal.y /= len; normal.z /= len; }
        rec.normal = normal;
        hit = true;
    }
    return hit;
}

} // namespace s21
