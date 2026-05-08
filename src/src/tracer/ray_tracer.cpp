#include "ray_tracer.h"
#include <cmath>
#include <glm/glm.hpp>
#include "matrix/s21_matrix_oop.h"
#include "common/transform.h"

namespace s21 {

RayTracer::RayTracer(Scene* scene, const Camera& camera, const LightManager& lightManager)
    : scene_(scene), camera_(camera), lightManager_(lightManager) {}

QImage RayTracer::Render(int width, int height, int samples) const {
    QImage image(width, height, QImage::Format_RGB32);
    auto pointLights = lightManager_.GetActiveLights();
    const auto& dirLight = lightManager_.GetDirectionalLight();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            glm::vec3 color(0.0f);
            for (int s = 0; s < samples; ++s) {
                float u = (x + (s % 2 == 0 ? 0.25f : 0.75f)) / width;
                float v = (y + (s % 2 == 0 ? 0.25f : 0.75f)) / height;
                Ray ray = GetCameraRay(static_cast<int>(u * width),
                                      static_cast<int>(v * height),
                                      width, height, camera_);
                HitRecord rec;
                if (scene_->TraceRay(ray, 0.001f, 1000.0f, rec)) {
                    color += Shade(ray, rec, pointLights, dirLight);
                } else {
                    // цвет фона (можно позже брать из настроек)
                    color += glm::vec3(0.1f, 0.1f, 0.15f);
                }
            }
            color /= float(samples);
            color = glm::clamp(color, 0.0f, 1.0f);
            image.setPixelColor(x, y, QColor::fromRgbF(color.r, color.g, color.b));
        }
    }
    return image;
}

Ray RayTracer::GetCameraRay(int x, int y, int width, int height, const Camera& camera) const {
    float aspect = float(width) / float(height);
    S21Matrix proj = camera.GetProjectionMatrix(aspect);
    S21Matrix view = camera.GetViewMatrix();
    S21Matrix invProj, invView;
    try {
        invProj = proj.InverseMatrix();
        invView = view.InverseMatrix();
    } catch (...) {
        return {Point{0,0,0}, Point{0,0,1}};
    }

    float ndcX = (2.0f * x / width - 1.0f);
    float ndcY = 1.0f - (2.0f * y / height);

    S21Matrix nearPoint(4, 1);
    nearPoint(0,0) = ndcX;
    nearPoint(1,0) = ndcY;
    nearPoint(2,0) = -1.0;
    nearPoint(3,0) = 1.0;

    S21Matrix eye = invProj * nearPoint;
    float w = eye(3,0);
    Point dir(
        static_cast<float>(eye(0,0) / w),
        static_cast<float>(eye(1,0) / w),
        static_cast<float>(eye(2,0) / w)
    );

    Point worldDir = TransformPoint(invView, dir);
    Point worldOrigin = TransformPoint(invView, Point{0,0,0});
    Point direction = worldDir - worldOrigin;
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    if (len > 1e-6f) {
        direction.x /= len; direction.y /= len; direction.z /= len;
    }
    return {worldOrigin, direction};
}

bool RayTracer::IsInShadow(const Point& point, const glm::vec3& lightDir,
                           float lightDist, float bias) const {
    Point shadowOrigin = {
        point.x + lightDir.x * bias,
        point.y + lightDir.y * bias,
        point.z + lightDir.z * bias
    };
    Point shadowDir = {lightDir.x, lightDir.y, lightDir.z};
    Ray shadowRay = {shadowOrigin, shadowDir};
    HitRecord tempRec;
    if (scene_->TraceRay(shadowRay, 0.001f, lightDist, tempRec))
        return true;
    return false;
}

glm::vec3 RayTracer::Shade(const Ray& ray, const HitRecord& hit,
                           const std::vector<LightSource>& pointLights,
                           const DirectionalLight& dirLight) const {
    const float kShadowBias = 0.01f;
    glm::vec3 ambient(0.15f, 0.15f, 0.15f);
    glm::vec3 color = ambient;
    glm::vec3 objectColor(0.8f, 0.8f, 0.8f);

    glm::vec3 normal(hit.normal.x, hit.normal.y, hit.normal.z);
    glm::vec3 viewDir = -glm::vec3(ray.direction.x, ray.direction.y, ray.direction.z);
    if (glm::dot(normal, viewDir) < 0) normal = -normal;

    for (const auto& light : pointLights) {
        if (!light.enabled) continue;
        glm::vec3 lightPos(light.position.x, light.position.y, light.position.z);
        glm::vec3 fragPos(hit.point.x, hit.point.y, hit.point.z);
        glm::vec3 lightDir = glm::normalize(lightPos - fragPos);
        float lightDist = glm::length(lightPos - fragPos);
        Point fragPoint = {hit.point.x, hit.point.y, hit.point.z};
        if (!IsInShadow(fragPoint, lightDir, lightDist, kShadowBias)) {
            float diff = std::max(glm::dot(normal, lightDir), 0.0f);
            color += glm::vec3(light.diffuse.r, light.diffuse.g, light.diffuse.b) * diff * objectColor;
        }
    }

    if (dirLight.enabled) {
        glm::vec3 dir = glm::normalize(glm::vec3(dirLight.direction.x, dirLight.direction.y, dirLight.direction.z));
        glm::vec3 lightDir = -dir;
        Point fragPoint = {hit.point.x, hit.point.y, hit.point.z};
        if (!IsInShadow(fragPoint, lightDir, 1000.0f, kShadowBias)) {
            float diff = std::max(glm::dot(normal, lightDir), 0.0f);
            color += glm::vec3(dirLight.color.r, dirLight.color.g, dirLight.color.b) * dirLight.intensity * diff * objectColor;
        }
    }
    return color;
}

} // namespace s21
