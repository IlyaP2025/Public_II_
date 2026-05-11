#include "cylinder_object.h"
#include <cmath>
#include <vector>
#include "common/point.h"
#include "scene/mesh.h"

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

    std::vector<Point> vertices;
    std::vector<unsigned int> indices;
    std::vector<Point> normals;

    float halfH = height_ / 2.0f;
    Point topCenter = {center_.x, center_.y + halfH, center_.z};
    Point bottomCenter = {center_.x, center_.y - halfH, center_.z};

    for (int i = 0; i <= precision; ++i) {
        float angle = 2.0f * M_PI * i / precision;
        float x = radius_ * std::cos(angle);
        float z = radius_ * std::sin(angle);
        Point normal = Point(std::cos(angle), 0.0f, std::sin(angle));

        vertices.push_back({center_.x + x, topCenter.y, center_.z + z});
        normals.push_back(normal);
        vertices.push_back({center_.x + x, bottomCenter.y, center_.z + z});
        normals.push_back(normal);
    }

    for (int i = 0; i < precision; ++i) {
        unsigned int top1 = i * 2;
        unsigned int top2 = (i + 1) * 2;
        unsigned int bot1 = i * 2 + 1;
        unsigned int bot2 = (i + 1) * 2 + 1;

        // Правильный порядок для внешних нормалей
        indices.push_back(top1);
        indices.push_back(bot1);
        indices.push_back(top2);

        indices.push_back(top2);
        indices.push_back(bot1);
        indices.push_back(bot2);
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

    return mesh;
}

} // namespace s21
