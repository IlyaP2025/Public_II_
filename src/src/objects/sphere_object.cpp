#include "sphere_object.h"
#include <cmath>
#include <vector>
#include "common/point.h"
#include "scene/mesh.h"
#include "common/debug.h"

namespace s21 {

SphereObject::SphereObject(float radius, const Point& center)
    : radius_(radius), center_(center) {
    SetName("Sphere");
}

bool SphereObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    Point oc = ray.origin - center_;
    float a = ray.direction.x * ray.direction.x +
              ray.direction.y * ray.direction.y +
              ray.direction.z * ray.direction.z;
    float b = 2.0f * (oc.x * ray.direction.x + oc.y * ray.direction.y + oc.z * ray.direction.z);
    float c = (oc.x * oc.x + oc.y * oc.y + oc.z * oc.z) - radius_ * radius_;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float sqrtd = std::sqrt(discriminant);
    float root = (-b - sqrtd) / (2.0f * a);
    if (root < t_min || root > t_max) {
        root = (-b + sqrtd) / (2.0f * a);
        if (root < t_min || root > t_max) return false;
    }

    rec.t = root;
    rec.point = ray.origin + Point(ray.direction.x * root,
                                  ray.direction.y * root,
                                  ray.direction.z * root);
    float invRadius = 1.0f / radius_;
    Point diff = rec.point - center_;
    rec.normal = Point(diff.x * invRadius, diff.y * invRadius, diff.z * invRadius);
    rec.material = material_;
    return true;
}

std::unique_ptr<Mesh> SphereObject::GenerateMesh(int precision) const {
    auto mesh = std::make_unique<Mesh>();
    if (precision < 3) precision = 3;

    std::vector<Point> baseVerts;
    std::vector<Point> baseNorms;
    std::vector<Point2D> baseUVs;

    for (int i = 0; i <= precision; ++i) {
        float theta = i * M_PI / precision;
        float sinTheta = std::sin(theta);
        float cosTheta = std::cos(theta);
        for (int j = 0; j <= precision; ++j) {
            float phi = j * 2.0f * M_PI / precision;
            float sinPhi = std::sin(phi);
            float cosPhi = std::cos(phi);

            Point p(
                center_.x + radius_ * sinTheta * cosPhi,
                center_.y + radius_ * cosTheta,
                center_.z + radius_ * sinTheta * sinPhi
            );
            baseVerts.push_back(p);
            baseNorms.push_back(Point(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi));
            baseUVs.push_back(Point2D(static_cast<float>(j) / precision,
                                     static_cast<float>(i) / precision));
        }
    }

    std::vector<Point> verts;
    std::vector<Point> norms;
    std::vector<Point2D> uvs;
    std::vector<unsigned int> tris;

    for (int i = 0; i < precision; ++i) {
        for (int j = 0; j < precision; ++j) {
            unsigned int first = i * (precision + 1) + j;
            unsigned int second = first + precision + 1;

            // Треугольник 1
            verts.push_back(baseVerts[first]);
            verts.push_back(baseVerts[first + 1]);
            verts.push_back(baseVerts[second]);
            norms.push_back(baseNorms[first]);
            norms.push_back(baseNorms[first + 1]);
            norms.push_back(baseNorms[second]);
            uvs.push_back(baseUVs[first]);
            uvs.push_back(baseUVs[first + 1]);
            uvs.push_back(baseUVs[second]);

            // Треугольник 2
            verts.push_back(baseVerts[first + 1]);
            verts.push_back(baseVerts[second + 1]);
            verts.push_back(baseVerts[second]);
            norms.push_back(baseNorms[first + 1]);
            norms.push_back(baseNorms[second + 1]);
            norms.push_back(baseNorms[second]);
            uvs.push_back(baseUVs[first + 1]);
            uvs.push_back(baseUVs[second + 1]);
            uvs.push_back(baseUVs[second]);
        }
    }

    for (size_t k = 0; k < verts.size(); k += 3) {
        tris.push_back(static_cast<unsigned int>(k));
        tris.push_back(static_cast<unsigned int>(k + 1));
        tris.push_back(static_cast<unsigned int>(k + 2));
    }

    mesh->SetVertices(verts);
    mesh->SetNormals(norms);
    mesh->SetUVs(uvs);
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
        // Вычислим нормаль первого треугольника
        const Point& a = v[0];
        const Point& b = v[1];
        const Point& c = v[2];
        float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        float nx = uy*vz - uz*vy;
        float ny = uz*vx - ux*vz;
        float nz = ux*vy - uy*vx;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
        DEBUG_PRINT("  Computed normal of first triangle: (" << nx << ", " << ny << ", " << nz << ")");
    }
    DEBUG_PRINT("=== End Mesh ===");

    return mesh;
}

} // namespace s21
