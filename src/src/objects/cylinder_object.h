#ifndef S21_OBJECTS_CYLINDER_OBJECT_H
#define S21_OBJECTS_CYLINDER_OBJECT_H

#include "analytic_object.h"

namespace s21 {

class CylinderObject : public AnalyticObject {
public:
    // Бесконечный цилиндр вдоль оси Y, усечённый по высоте
    CylinderObject(float radius, float height, const Point& center = {0,0,0});

    bool Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override;
    std::unique_ptr<Mesh> GenerateMesh(int precision = 32) const override;

    float GetRadius() const { return radius_; }
    float GetHeight() const { return height_; }
    Point GetCenter() const { return center_; }

private:
    float radius_;
    float height_;
    Point center_;
};

} // namespace s21

#endif
