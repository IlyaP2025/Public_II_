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
        apex, base[0], base[1], base[2], base[3]
    };
    mesh->SetVertices(verts);
    mesh->SetTriangles({
        0,1,2,  0,2,3,  0,3,4,  0,4,1,
        1,3,2,  1,4,3  // основание (два треугольника)
    });
    mesh->SetNormals(std::vector<Point>(5, Point(0,1,0)));
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
