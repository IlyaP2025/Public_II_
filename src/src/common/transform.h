#ifndef S21_TRANSFORM_H
#define S21_TRANSFORM_H

#include "point.h"
#include "matrix/s21_matrix_oop.h"

namespace s21 {

class Transform {
 public:
  Transform();
  Transform(const Point& position, const Point& rotation, const Point& scale);

  void SetPosition(const Point& pos);
  Point GetPosition() const;

  void SetRotation(const Point& rot);
  Point GetRotation() const;

  void SetScale(const Point& scale);
  Point GetScale() const;

  S21Matrix GetModelMatrix() const;

 private:
  Point position_;
  Point rotation_;
  Point scale_;
};

// Вспомогательная функция для умножения матрицы на точку
Point TransformPoint(const S21Matrix& m, const Point& p);

}  // namespace s21

#endif  // S21_TRANSFORM_H
