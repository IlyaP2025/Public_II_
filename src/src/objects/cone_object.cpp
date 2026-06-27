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
    std::vector<Point2D> uvs;

    float halfH = height_ / 2.0f;
    Point apex = {center_.x, center_.y + halfH, center_.z};
    Point baseCenter = {center_.x, center_.y - halfH, center_.z};

    // Боковая поверхность
    for (int i = 0; i < precision; ++i) {
        float angle1 = 2.0f * M_PI * i / precision;
        float angle2 = 2.0f * M_PI * (i + 1) / precision;
        float x1 = radius_ * std::cos(angle1);
        float z1 = radius_ * std::sin(angle1);
        float x2 = radius_ * std::cos(angle2);
        float z2 = radius_ * std::sin(angle2);
        Point base1 = {center_.x + x1, baseCenter.y, center_.z + z1};
        Point base2 = {center_.x + x2, baseCenter.y, center_.z + z2};

        Point u = {base2.x - base1.x, base2.y - base1.y, base2.z - base1.z};
        Point v = {apex.x - base1.x, apex.y - base1.y, apex.z - base1.z};
        Point n = {u.y*v.z - u.z*v.y, u.z*v.x - u.x*v.z, u.x*v.y - u.y*v.x};
        float len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
        if (len > 1e-6f) { n.x /= len; n.y /= len; n.z /= len; }
        if (n.y < 0) { n.x = -n.x; n.y = -n.y; n.z = -n.z; }

        float u1 = static_cast<float>(i) / precision;
        float u2 = static_cast<float>(i + 1) / precision;

        vertices.push_back(apex);   normals.push_back(n);
        vertices.push_back(base1);  normals.push_back(n);
        vertices.push_back(base2);  normals.push_back(n);

        uvs.emplace_back(0.5f, 1.0f);
        uvs.emplace_back(u1, 0.0f);
        uvs.emplace_back(u2, 0.0f);
    }

    // Нижнее основание
    Point normBottom(0.0f, -1.0f, 0.0f);
    for (int i = 0; i < precision; ++i) {
        float angle1 = 2.0f * M_PI * i / precision;
        float angle2 = 2.0f * M_PI * (i + 1) / precision;
        float x1 = radius_ * std::cos(angle1);
        float z1 = radius_ * std::sin(angle1);
        float x2 = radius_ * std::cos(angle2);
        float z2 = radius_ * std::sin(angle2);

        Point v1 = {center_.x + x1, baseCenter.y, center_.z + z1};
        Point v2 = {center_.x + x2, baseCenter.y, center_.z + z2};

        vertices.push_back(baseCenter); normals.push_back(normBottom);
        vertices.push_back(v1);         normals.push_back(normBottom);
        vertices.push_back(v2);         normals.push_back(normBottom);

        uvs.emplace_back(0.5f, 0.5f);
        uvs.emplace_back(0.5f + 0.5f * std::cos(angle1), 0.5f + 0.5f * std::sin(angle1));
        uvs.emplace_back(0.5f + 0.5f * std::cos(angle2), 0.5f + 0.5f * std::sin(angle2));
    }

    for (size_t k = 0; k < vertices.size(); k += 3) {
        indices.push_back(static_cast<unsigned int>(k));
        indices.push_back(static_cast<unsigned int>(k + 1));
        indices.push_back(static_cast<unsigned int>(k + 2));
    }

    mesh->SetVertices(vertices);
    mesh->SetNormals(normals);
    mesh->SetTriangles(indices);
    mesh->SetFlatNormals(normals);
    mesh->SetSmoothNormals(normals);
    mesh->SetUVs(uvs);
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
