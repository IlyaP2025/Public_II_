#ifndef S21_OBJECTS_PLANE_OBJECT_H
#define S21_OBJECTS_PLANE_OBJECT_H

#include "analytic_object.h"

namespace s21 {

class PlaneObject : public AnalyticObject {
public:
    PlaneObject(const Point& point, const Point& normal); // плоскость задана точкой и нормалью

    bool Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override;
    std::unique_ptr<Mesh> GenerateMesh(int precision = 1) const override;

    Point GetPoint() const { return point_; }
    Point GetNormal() const { return normal_; }

private:
    Point point_;
    Point normal_;
};

} // namespace s21

#endif
