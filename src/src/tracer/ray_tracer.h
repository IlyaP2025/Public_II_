#ifndef S21_TRACER_RAY_TRACER_H
#define S21_TRACER_RAY_TRACER_H

#include <QImage>
#include "scene/scene.h"
#include "scene/camera.h"
#include "common/lighting.h"
#include "common/ray.h"

namespace s21 {

class RayTracer {
public:
    RayTracer(Scene* scene, const Camera& camera, const LightManager& lightManager);

    QImage Render(int width, int height, int samples = 1) const;

private:
    glm::vec3 Shade(const Ray& ray, const HitRecord& hit,
                    const std::vector<LightSource>& pointLights,
                    const DirectionalLight& dirLight) const;
    Ray GetCameraRay(int x, int y, int width, int height, const Camera& camera) const;

    Scene* scene_;
    const Camera& camera_;
    const LightManager& lightManager_;
};

} // namespace s21

#endif
