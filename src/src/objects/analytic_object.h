#ifndef S21_OBJECTS_ANALYTIC_OBJECT_H
#define S21_OBJECTS_ANALYTIC_OBJECT_H

#include <memory>
#include "common/point.h"
#include "common/material.h"
#include "scene/scene_object.h"
#include "common/ray.h"

namespace s21 {

struct HitRecord {
    float t;
    Point point;
    Point normal;
    Material material;
};

class AnalyticObject : public SceneObject {
public:
    AnalyticObject() = default;
    virtual ~AnalyticObject() = default;

    // Проверка пересечения луча с объектом
    virtual bool Hit(const Ray& ray, float t_min, float t_max, HitRecord& rec) const = 0;

    // Генерация полигональной сетки для отображения
    virtual std::unique_ptr<Mesh> GenerateMesh(int precision = 32) const = 0;

    void SetMaterial(const Material& mat) { material_ = mat; }
    const Material& GetMaterial() const { return material_; }

protected:
    Material material_;
};

} // namespace s21

#endif
