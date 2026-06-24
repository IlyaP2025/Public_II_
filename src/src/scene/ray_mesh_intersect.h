#ifndef S21_RAY_MESH_INTERSECT_H
#define S21_RAY_MESH_INTERSECT_H

namespace s21 {

class Mesh;
struct Ray;
struct HitRecord;

bool RayMeshIntersect(const Mesh& mesh, const Ray& ray,
                      float t_min, float t_max, HitRecord& rec);

} // namespace s21

#endif // S21_RAY_MESH_INTERSECT_H
