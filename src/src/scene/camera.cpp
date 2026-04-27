#include "camera.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace s21 {

Camera::Camera()
    : position_{0, 0, 5},
      target_{0, 0, 0},
      up_{0, 1, 0},
      projectionType_(Settings::ProjectionType::Orthographic),
      zoomFactor_(1.0f),
      nearPlane_(0.1f),
      farPlane_(100.0f) {}

void Camera::SetPosition(float x, float y, float z) {
  position_[0] = x;
  position_[1] = y;
  position_[2] = z;
}

void Camera::SetTarget(float x, float y, float z) {
  target_[0] = x;
  target_[1] = y;
  target_[2] = z;
}

void Camera::SetUp(float x, float y, float z) {
  up_[0] = x;
  up_[1] = y;
  up_[2] = z;
}

void Camera::SetProjectionType(Settings::ProjectionType type) {
  projectionType_ = type;
}

void Camera::SetZoomFactor(float factor) {
  // Расширяем допустимый диапазон, чтобы вмещать очень большие или очень
  // маленькие объекты
  zoomFactor_ = std::clamp(factor, 0.0001f, 10000.0f);
}

void Camera::SetNearPlane(float nearPlane) { nearPlane_ = nearPlane; }

void Camera::SetFarPlane(float farPlane) { farPlane_ = farPlane; }

S21Matrix Camera::GetViewMatrix() const {
  float fx = target_[0] - position_[0];
  float fy = target_[1] - position_[1];
  float fz = target_[2] - position_[2];
  float fLen = std::sqrt(fx * fx + fy * fy + fz * fz);
  if (fLen > 0) {
    fx /= fLen;
    fy /= fLen;
    fz /= fLen;
  }

  float rx = up_[1] * fz - up_[2] * fy;
  float ry = up_[2] * fx - up_[0] * fz;
  float rz = up_[0] * fy - up_[1] * fx;
  float rLen = std::sqrt(rx * rx + ry * ry + rz * rz);
  if (rLen > 0) {
    rx /= rLen;
    ry /= rLen;
    rz /= rLen;
  }

  float ux = fy * rz - fz * ry;
  float uy = fz * rx - fx * rz;
  float uz = fx * ry - fy * rx;

  S21Matrix view(4, 4);
  view(0, 0) = rx;
  view(0, 1) = ry;
  view(0, 2) = rz;
  view(0, 3) = -(rx * position_[0] + ry * position_[1] + rz * position_[2]);
  view(1, 0) = ux;
  view(1, 1) = uy;
  view(1, 2) = uz;
  view(1, 3) = -(ux * position_[0] + uy * position_[1] + uz * position_[2]);
  view(2, 0) = -fx;
  view(2, 1) = -fy;
  view(2, 2) = -fz;
  view(2, 3) = (fx * position_[0] + fy * position_[1] + fz * position_[2]);
  view(3, 0) = 0;
  view(3, 1) = 0;
  view(3, 2) = 0;
  view(3, 3) = 1;

  return view;
}

S21Matrix Camera::GetProjectionMatrix(float aspect) const {
  if (projectionType_ == Settings::ProjectionType::Orthographic) {
    float left = -5.0f * zoomFactor_;
    float right = 5.0f * zoomFactor_;
    float bottom = -5.0f * zoomFactor_;
    float top = 5.0f * zoomFactor_;
    float nearPlane = nearPlane_;
    float farPlane = farPlane_;
    S21Matrix proj(4, 4);
    proj(0, 0) = 2.0f / (right - left);
    proj(1, 1) = 2.0f / (top - bottom);
    proj(2, 2) = -2.0f / (farPlane - nearPlane);
    proj(0, 3) = -(right + left) / (right - left);
    proj(1, 3) = -(top + bottom) / (top - bottom);
    proj(2, 3) = -(farPlane + nearPlane) / (farPlane - nearPlane);
    proj(3, 3) = 1.0f;
    return proj;
  } else {
    float fov = 45.0f * static_cast<float>(std::numbers::pi) / 180.0f;
    float nearPlane = nearPlane_;
    float farPlane = farPlane_;
    float top = nearPlane * std::tan(fov / 2.0f);
    float bottom = -top;
    float right = top * aspect;
    float left = -right;

    S21Matrix proj(4, 4);
    proj(0, 0) = 2.0f * nearPlane / (right - left);
    proj(0, 2) = 0.0f;  // исправлено: для симметричного frustum
    proj(1, 1) = 2.0f * nearPlane / (top - bottom);
    proj(1, 2) = 0.0f;  // исправлено
    proj(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
    proj(2, 3) = -2.0f * farPlane * nearPlane / (farPlane - nearPlane);
    proj(3, 2) = -1.0f;
    proj(3, 3) = 0.0f;
    return proj;
  }
}

void Camera::Rotate(float deltaX, float deltaY) {
  const float degToRad = static_cast<float>(std::numbers::pi) / 180.0f;
  float dx = position_[0] - target_[0];
  float dy = position_[1] - target_[1];
  float dz = position_[2] - target_[2];
  float radius = std::sqrt(dx * dx + dy * dy + dz * dz);
  const float minRadius = 0.01f;  // увеличенный порог
  if (radius < minRadius) return;

  float theta = std::atan2(dz, dx);
  float phi = std::asin(dy / radius);  // было: без защиты

  // Исправление: зажимаем значение перед asin
  float ratio = dy / radius;
  if (ratio > 1.0f) ratio = 1.0f;
  if (ratio < -1.0f) ratio = -1.0f;
  phi = std::asin(ratio);

  theta += deltaX * degToRad;
  phi += deltaY * degToRad;

  const float maxPhi = 89.0f * degToRad;
  phi = std::clamp(phi, -maxPhi, maxPhi);

  float cosPhi = std::cos(phi);
  position_[0] = target_[0] + radius * cosPhi * std::cos(theta);
  position_[1] = target_[1] + radius * std::sin(phi);
  position_[2] = target_[2] + radius * cosPhi * std::sin(theta);
}

void Camera::Zoom(float delta) {
  // delta > 0 – приближение (колёсико вверх), delta < 0 – отдаление
  float dx = position_[0] - target_[0];
  float dy = position_[1] - target_[1];
  float dz = position_[2] - target_[2];
  float radius = std::sqrt(dx * dx + dy * dy + dz * dz);

  const float minRadius = 0.5f;
  const float maxRadius = 50000.0f;  // увеличен для огромных объектов

  // Относительное изменение: при delta = 1 радиус умножается на 0.9
  // (приближение на 10%) при delta = -1 радиус умножается на 1.1 (отдаление на
  // 10%)
  float factor = 1.0f;
  if (delta > 0) {
    factor = 1.0f - delta * 0.1f;  // например, 10% за единицу delta
    if (factor < 0.001f) factor = 0.001f;
  } else if (delta < 0) {
    factor = 1.0f - delta * 0.1f;  // delta отрицательный, factor > 1
    if (factor > 1000.0f) factor = 1000.0f;  // ограничиваем максимальный рост
  }

  float newRadius = radius * factor;
  newRadius = std::clamp(newRadius, minRadius, maxRadius);

  if (radius < 1e-6f) return;

  float scale = newRadius / radius;
  position_[0] = target_[0] + dx * scale;
  position_[1] = target_[1] + dy * scale;
  position_[2] = target_[2] + dz * scale;

  if (projectionType_ == Settings::ProjectionType::Orthographic) {
    // Для ортографической проекции также меняем zoomFactor пропорционально
    float newZoom = zoomFactor_ * factor;
    SetZoomFactor(newZoom);  // используем сеттер с ограничениями
  }
}

void Camera::Pan(float deltaX, float deltaY) {
  float dirX = target_[0] - position_[0];
  float dirY = target_[1] - position_[1];
  float dirZ = target_[2] - position_[2];
  float len = std::sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ);
  if (len < 1e-6f) return;
  dirX /= len;
  dirY /= len;
  dirZ /= len;

  float upX = up_[0], upY = up_[1], upZ = up_[2];

  float rightX = dirY * upZ - dirZ * upY;
  float rightY = dirZ * upX - dirX * upZ;
  float rightZ = dirX * upY - dirY * upX;
  float rightLen =
      std::sqrt(rightX * rightX + rightY * rightY + rightZ * rightZ);
  if (rightLen < 1e-6f) return;
  rightX /= rightLen;
  rightY /= rightLen;
  rightZ /= rightLen;

  float upActualX = rightY * dirZ - rightZ * dirY;
  float upActualY = rightZ * dirX - rightX * dirZ;
  float upActualZ = rightX * dirY - rightY * dirX;

  float scale = len * 0.001f;

  target_[0] += (rightX * deltaX + upActualX * deltaY) * scale;
  target_[1] += (rightY * deltaX + upActualY * deltaY) * scale;
  target_[2] += (rightZ * deltaX + upActualZ * deltaY) * scale;

  position_[0] += (rightX * deltaX + upActualX * deltaY) * scale;
  position_[1] += (rightY * deltaX + upActualY * deltaY) * scale;
  position_[2] += (rightZ * deltaX + upActualZ * deltaY) * scale;
}

}  // namespace s21
