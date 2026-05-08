#ifndef S21_OBJECTS_CUBE_OBJECT_H
#define S21_OBJECTS_CUBE_OBJECT_H

#include "analytic_object.h"

namespace s21 {

class CubeObject : public AnalyticObject {
public:
    CubeObject(float size, const Point& center = {0,0,0});

    bool Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const override;
    std::unique_ptr<Mesh> GenerateMesh(int precision = 1) const override;

private:
    float size_;
    Point center_;
};

} // namespace s21

#endif
