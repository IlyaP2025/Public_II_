#include "sphere_object.h"
#include <cmath>
#include <vector>
#include "common/point.h"
#include "scene/mesh.h"

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
    rec.normal = (rec.point - center_) * (1.0f / radius_);
    rec.material = material_;
    return true;
}

std::unique_ptr<Mesh> SphereObject::GenerateMesh(int precision) const {
    auto mesh = std::make_unique<Mesh>();
    if (precision < 3) precision = 3;

    std::vector<Point> vertices;
    std::vector<unsigned int> indices;
    std::vector<Point> normals;
    std::vector<Point2D> uvs;

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
            vertices.push_back(p);
            normals.push_back(Point(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi));
            uvs.push_back(Point2D(static_cast<float>(j) / precision,
                                 static_cast<float>(i) / precision));
        }
    }

    for (int i = 0; i < precision; ++i) {
        for (int j = 0; j < precision; ++j) {
            unsigned int first = i * (precision + 1) + j;
            unsigned int second = first + precision + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    mesh->SetVertices(vertices);
    mesh->SetNormals(normals);
    mesh->SetUVs(uvs);
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
