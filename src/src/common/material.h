#ifndef S21_COMMON_MATERIAL_H
#define S21_COMMON_MATERIAL_H

namespace s21 {

struct Material {
    float transparency = 0.0f;   // 0 - полностью непрозрачный, 1 - полностью прозрачный
    float ior = 1.0f;            // коэффициент преломления (для прозрачных объектов)
    float reflectivity = 0.0f;   // 0 - матовый, 1 - зеркало
    float roughness = 0.0f;      // шероховатость поверхности (0 - гладкая, 1 - шершавая)
};

} // namespace s21

#endif
