#include "pyramid_object.h"
#include "scene/mesh.h"
#include "common/debug.h"

namespace s21 {

PyramidObject::PyramidObject(float base, float height, const Point& center)
    : base_(base), height_(height), center_(center) {
    SetName("Pyramid");
}

bool PyramidObject::Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    (void)ray; (void)t_min; (void)t_max; (void)rec;
    return false;
}

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
        // Треугольник: apex, a, bPt
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

    // Отладка
    DEBUG_PRINT("=== Mesh generated: " << GetName().c_str() << " ===");
    DEBUG_PRINT("  Vertices: " << mesh->GetVertices().size());
    DEBUG_PRINT("  Normals: " << mesh->GetNormals().size());
    DEBUG_PRINT("  Triangles: " << mesh->GetTriangles().size() / 3);
    if (mesh->GetVertices().size() >= 3) {
        const auto& v = mesh->GetVertices();
        DEBUG_PRINT("  First vertex: (" << v[0].x << ", " << v[0].y << ", " << v[0].z << ")");
        DEBUG_PRINT("  Last vertex: (" << v.back().x << ", " << v.back().y << ", " << v.back().z << ")");
        const Point& a = v[0], &b = v[1], &c = v[2];
        float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        float nx = uy*vz - uz*vy, ny = uz*vx - ux*vz, nz = ux*vy - uy*vx;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
        DEBUG_PRINT("  Computed normal of first triangle: (" << nx << ", " << ny << ", " << nz << ")");
    }
    DEBUG_PRINT("=== End Mesh ===");

    return mesh;
}

} // namespace s21
