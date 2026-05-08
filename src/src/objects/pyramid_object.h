#ifndef S21_OBJECTS_PYRAMID_OBJECT_H
#define S21_OBJECTS_PYRAMID_OBJECT_H

#include "analytic_object.h"

namespace s21 {

class PyramidObject : public AnalyticObject {
public:
    PyramidObject(float base, float height, const Point& center = {0,0,0});

    bool Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override;
    std::unique_ptr<Mesh> GenerateMesh(int precision = 1) const override;

private:
    float base_;
    float height_;
    Point center_;
};

} // namespace s21

#endif
