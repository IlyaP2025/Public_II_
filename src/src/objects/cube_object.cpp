#include "cube_object.h"
#include "scene/mesh.h"

namespace s21 {

CubeObject::CubeObject(float size, const Point& center) : size_(size), center_(center) {
    SetName("Cube");
}

bool CubeObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    // ... (без изменений) ...
}

std::unique_ptr<Mesh> CubeObject::GenerateMesh(int /*precision*/) const {
    auto mesh = std::make_unique<Mesh>();
    float h = size_ / 2.0f;
    std::vector<Point> verts = {
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        // Дублируем вершины для корректных нормалей
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x - h, center_.y - h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x - h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z - h},
        {center_.x + h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y + h, center_.z + h},
        {center_.x - h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z - h},
        {center_.x + h, center_.y - h, center_.z + h},
        {center_.x - h, center_.y - h, center_.z + h},
    };
    mesh->SetVertices(verts);
    mesh->SetTriangles({
        0,1,2, 0,2,3,    // front
        4,5,6, 4,6,7,    // back
        8,9,10, 8,10,11, // left
        12,13,14, 12,14,15, // right
        16,17,18, 16,18,19, // top
        20,21,22, 20,22,23  // bottom
    });
    // Нормали для каждой грани
    std::vector<Point> norms = {
        {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, // front
        {0, 0,  1}, {0, 0,  1}, {0, 0,  1}, {0, 0,  1}, // back
        {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, // left
        {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0},     // right
        {0, 1, 0}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0},     // top
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}  // bottom
    };
    mesh->SetNormals(norms);
    mesh->ComputeBoundingSphere();
    // Генерация ребер
    std::vector<Edge> edges;
    const auto& tris = mesh->GetTriangles();
    for (size_t i = 0; i < tris.size(); i += 3) {
        edges.push_back(Edge{tris[i], tris[i+1]});
        edges.push_back(Edge{tris[i+1], tris[i+2]});
        edges.push_back(Edge{tris[i+2], tris[i]});
    }
    mesh->SetEdges(edges);
    return mesh;
}

} // namespace s21
