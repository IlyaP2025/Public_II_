#include "pyramid_object.h"
#include "scene/mesh.h"

namespace s21 {

PyramidObject::PyramidObject(float base, float height, const Point& center)
    : base_(base), height_(height), center_(center) {
    SetName("Pyramid");
}

bool PyramidObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    // Упрощённо: трассируем как пересечение с 4 треугольниками и квадратным основанием
    // Здесь для краткости опускаем, можно реализовать позже
    (void)ray; (void)t_min; (void)t_max; (void)rec;
    return false;
}

std::unique_ptr<Mesh> PyramidObject::GenerateMesh(int /*precision*/) const {
    auto mesh = std::make_unique<Mesh>();
    float h = height_ / 2.0f;
    float b = base_ / 2.0f;
    Point apex = {center_.x, center_.y + h, center_.z};
    Point base[4] = {
        {center_.x - b, center_.y - h, center_.z - b},
        {center_.x + b, center_.y - h, center_.z - b},
        {center_.x + b, center_.y - h, center_.z + b},
        {center_.x - b, center_.y - h, center_.z + b}
    };

    std::vector<Point> verts = {
        apex, base[0], base[1],
        apex, base[1], base[2],
        apex, base[2], base[3],
        apex, base[3], base[0],
        base[0], base[1], base[3],
        base[1], base[2], base[3]
    };

    auto computeNormal = [](const Point& a, const Point& b, const Point& c) {
        Point u = {b.x - a.x, b.y - a.y, b.z - a.z};
        Point v = {c.x - a.x, c.y - a.y, c.z - a.z};
        Point n = {
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        };
        float len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
        return Point{n.x/len, n.y/len, n.z/len};
    };

    std::vector<Point> norms;
    for (int i = 0; i < 4; ++i) {
        Point n = computeNormal(verts[i*3], verts[i*3+1], verts[i*3+2]);
        if (n.y < 0) n = Point{-n.x, -n.y, -n.z};
        norms.push_back(n);
        norms.push_back(n);
        norms.push_back(n);
    }
    norms.push_back({0, -1, 0}); norms.push_back({0, -1, 0}); norms.push_back({0, -1, 0});
    norms.push_back({0, -1, 0}); norms.push_back({0, -1, 0}); norms.push_back({0, -1, 0});

    mesh->SetVertices(verts);
    mesh->SetNormals(norms);
    mesh->SetTriangles({
        0,1,2,  3,4,5,  6,7,8,  9,10,11,
        12,13,14,  15,16,17
    });
    mesh->ComputeBoundingSphere();

    std::vector<Edge> edges;
    const auto& tri = mesh->GetTriangles();
    for (size_t i = 0; i < tri.size(); i += 3) {
        edges.push_back(Edge{tri[i], tri[i+1]});
        edges.push_back(Edge{tri[i+1], tri[i+2]});
        edges.push_back(Edge{tri[i+2], tri[i]});
    }
    mesh->SetEdges(edges);
    return mesh;
}

} // namespace s21
