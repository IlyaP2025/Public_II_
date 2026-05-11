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

    auto computeNormal = [](const Point& a, const Point& b, const Point& c) -> Point {
        Point u = {b.x - a.x, b.y - a.y, b.z - a.z};
        Point v = {c.x - a.x, c.y - a.y, c.z - a.z};
        Point n = {
            u.y * v.z - u.z * v.y,
            u.z * v.x - u.x * v.z,
            u.x * v.y - u.y * v.x
        };
        float len = std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
        if (len < 1e-6f) return Point{0,1,0};
        return Point{n.x/len, n.y/len, n.z/len};
    };

    std::vector<Point> norms;
    for (int i = 0; i < 4; ++i) {
        Point n = computeNormal(verts[i*3], verts[i*3+2], verts[i*3+1]);
        if (n.y < 0) n = Point{-n.x, -n.y, -n.z};
        norms.insert(norms.end(), 3, n);
    }
    norms.insert(norms.end(), 6, {0, -1, 0});

    mesh->SetVertices(verts);
    mesh->SetNormals(norms);
    mesh->SetTriangles({
        0,2,1,  3,5,4,  6,8,7,  9,11,10,
        12,14,13,  15,17,16
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

    // Отладка
    DEBUG_PRINT("=== Mesh generated: " << GetName().c_str() << " ===");
    DEBUG_PRINT("  Vertices: " << mesh->GetVertices().size());
    if (!mesh->GetVertices().empty()) {
        const auto& v = mesh->GetVertices();
        DEBUG_PRINT("  First vertex: (" << v[0].x << ", " << v[0].y << ", " << v[0].z << ")");
        DEBUG_PRINT("  Last vertex:  (" << v.back().x << ", " << v.back().y << ", " << v.back().z << ")");
    }
    const auto& norms_data = mesh->GetNormals();
    DEBUG_PRINT("  Normals: " << norms_data.size());
    if (!norms_data.empty()) {
        DEBUG_PRINT("  First normal: (" << norms_data[0].x << ", " << norms_data[0].y << ", " << norms_data[0].z << ")");
        if (norms_data.size() >= 4) {
            DEBUG_PRINT("  Last normal:  (" << norms_data.back().x << ", " << norms_data.back().y << ", " << norms_data.back().z << ")");
        }
    }
    const auto& tris = mesh->GetTriangles();
    DEBUG_PRINT("  Triangles: " << tris.size() / 3);
    if (tris.size() >= 6) {
        DEBUG_PRINT("  First triangle: " << tris[0] << ", " << tris[1] << ", " << tris[2]);
        if (mesh->GetVertices().size() > tris[2]) {
            const Point& a = mesh->GetVertices()[tris[0]];
            const Point& b = mesh->GetVertices()[tris[1]];
            const Point& c = mesh->GetVertices()[tris[2]];
            float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
            float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
            float nx = uy*vz - uz*vy;
            float ny = uz*vx - ux*vz;
            float nz = ux*vy - uy*vx;
            float len = std::sqrt(nx*nx + ny*ny + nz*nz);
            if (len > 1e-6f) { nx /= len; ny /= len; nz /= len; }
            DEBUG_PRINT("  Computed normal of first triangle: (" << nx << ", " << ny << ", " << nz << ")");
        }
    }
    DEBUG_PRINT("=== End Mesh ===");

    return mesh;
}

} // namespace s21
