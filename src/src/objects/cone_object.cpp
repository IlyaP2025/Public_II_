#include "cone_object.h"
#include <cmath>
#include <vector>
#include "common/point.h"
#include "scene/mesh.h"

namespace s21 {

ConeObject::ConeObject(float radius, float height, const Point& center)
    : radius_(radius), height_(height), center_(center) {
    SetName("Cone");
}

bool ConeObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    // Вершина конуса (вверху)
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

    // Вершина конуса
    unsigned int apexIndex = 0;
    vertices.push_back(apex);
    normals.push_back(Point(0, 1, 0));

    // Кольцо основания
    for (int i = 0; i <= precision; ++i) {
        float angle = 2.0f * M_PI * i / precision;
        float x = radius_ * std::cos(angle);
        float z = radius_ * std::sin(angle);
        vertices.push_back({center_.x + x, baseCenter.y, center_.z + z});
        // Нормаль для точки основания
        Point n = Point(x, 0, z);
        float ln = std::sqrt(n.x * n.x + n.z * n.z);
        if (ln > 1e-6f) {
            n.x /= ln; n.z /= ln;
        }
        normals.push_back(n);
    }

    // Боковые треугольники
    for (unsigned int i = 0; i < static_cast<unsigned int>(precision); ++i) {
        indices.push_back(apexIndex);
        indices.push_back(i + 1);
        indices.push_back(i + 2);
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
