#include "transform.h"

#include <cmath>
#include <numbers>

namespace s21 {

namespace {
const double DEG_TO_RAD = std::numbers::pi / 180.0;
}

Transform::Transform() : position_(), rotation_(), scale_(1.0f, 1.0f, 1.0f) {}

Transform::Transform(const Point& position, const Point& rotation,
                     const Point& scale)
    : position_(position), rotation_(rotation), scale_(scale) {}

void Transform::SetPosition(const Point& pos) { position_ = pos; }
Point Transform::GetPosition() const { return position_; }

void Transform::SetRotation(const Point& rot) { rotation_ = rot; }
Point Transform::GetRotation() const { return rotation_; }

void Transform::SetScale(const Point& scale) { scale_ = scale; }
Point Transform::GetScale() const { return scale_; }

S21Matrix Transform::GetModelMatrix() const {
  // Матрица перемещения
  S21Matrix translation(4, 4);
  translation(0, 0) = 1.0;
  translation(1, 1) = 1.0;
  translation(2, 2) = 1.0;
  translation(3, 3) = 1.0;
  translation(0, 3) = position_.x;
  translation(1, 3) = position_.y;
  translation(2, 3) = position_.z;

  double rx = rotation_.x * DEG_TO_RAD;
  double ry = rotation_.y * DEG_TO_RAD;
  double rz = rotation_.z * DEG_TO_RAD;

  // Матрица поворота вокруг X
  S21Matrix rotX(4, 4);
  rotX(0, 0) = 1.0;
  rotX(1, 1) = std::cos(rx);
  rotX(1, 2) = -std::sin(rx);
  rotX(2, 1) = std::sin(rx);
  rotX(2, 2) = std::cos(rx);
  rotX(3, 3) = 1.0;

  // Матрица поворота вокруг Y
  S21Matrix rotY(4, 4);
  rotY(0, 0) = std::cos(ry);
  rotY(0, 2) = std::sin(ry);
  rotY(1, 1) = 1.0;
  rotY(2, 0) = -std::sin(ry);
  rotY(2, 2) = std::cos(ry);
  rotY(3, 3) = 1.0;

  // Матрица поворота вокруг Z
  S21Matrix rotZ(4, 4);
  rotZ(0, 0) = std::cos(rz);
  rotZ(0, 1) = -std::sin(rz);
  rotZ(1, 0) = std::sin(rz);
  rotZ(1, 1) = std::cos(rz);
  rotZ(2, 2) = 1.0;
  rotZ(3, 3) = 1.0;

  // Матрица масштабирования с защитой от нуля
  S21Matrix scale(4, 4);
  float sx = scale_.x;
  float sy = scale_.y;
  float sz = scale_.z;
  if (sx == 0.0f) sx = 1e-6f;
  if (sy == 0.0f) sy = 1e-6f;
  if (sz == 0.0f) sz = 1e-6f;
  scale(0, 0) = sx;
  scale(1, 1) = sy;
  scale(2, 2) = sz;
  scale(3, 3) = 1.0;

  // Комбинированное вращение: Z * Y * X
  S21Matrix rotation = rotZ * rotY * rotX;

  // Комбинированная матрица: перемещение * поворот * масштаб
  S21Matrix result = translation * rotation * scale;

  return result;
}

}  // namespace s21
