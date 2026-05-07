#ifndef S21_OBJECTS_SPHERE_OBJECT_H
#define S21_OBJECTS_SPHERE_OBJECT_H

#include "analytic_object.h"
#include <memory>

namespace s21 {

class SphereObject : public AnalyticObject {
public:
    SphereObject(float radius = 1.0f, const Point& center = {0,0,0});

    bool Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override;
    std::unique_ptr<Mesh> GenerateMesh(int precision = 32) const override;

    float GetRadius() const { return radius_; }
    void SetRadius(float r) { radius_ = r; }
    Point GetCenter() const { return center_; }
    void SetCenter(const Point& c) { center_ = c; }

private:
    float radius_;
    Point center_;
};

} // namespace s21

#endif
