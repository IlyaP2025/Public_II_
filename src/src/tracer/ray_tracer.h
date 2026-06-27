#ifndef S21_TRACER_RAY_TRACER_H
#define S21_TRACER_RAY_TRACER_H

#include <QImage>
#include <functional>
#include "scene/scene.h"
#include "scene/camera.h"
#include "common/lighting.h"
#include "common/ray.h"

namespace s21 {

class RayTracer {
public:
    RayTracer(Scene* scene, const Camera& camera, const LightManager& lightManager);

    // Параллельный рендеринг (без прогресса)
    QImage Render(int width, int height, int samples = 1) const;

    // Последовательный рендеринг с обратным вызовом прогресса (percent 0–100)
    QImage RenderWithProgress(int width, int height, int samples,
                              std::function<void(int)> progressCallback) const;

private:
    glm::vec3 Shade(const Ray& ray, const HitRecord& hit,
                    const std::vector<LightSource>& pointLights,
                    const DirectionalLight& dirLight) const;
    Ray GetCameraRay(int x, int y, int width, int height, const Camera& camera) const;
    bool IsInShadow(const Point& point, const glm::vec3& lightDir,
                    float lightDist, float bias) const;

    Scene* scene_;
    const Camera& camera_;
    const LightManager& lightManager_;
};

} // namespace s21

#endif
