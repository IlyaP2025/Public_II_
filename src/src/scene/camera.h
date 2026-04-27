#ifndef S21_CAMERA_H
#define S21_CAMERA_H

#include <algorithm>

#include "common/point.h"
#include "matrix/s21_matrix_oop.h"
#include "settings/settings.h"

namespace s21 {

class Camera {
 public:
  Camera();

  void SetPosition(float x, float y, float z);
  void SetTarget(float x, float y, float z);
  void SetUp(float x, float y, float z);
  void SetProjectionType(Settings::ProjectionType type);
  void SetZoomFactor(float factor);

  void SetNearPlane(float nearPlane);
  void SetFarPlane(float farPlane);

  float GetPositionX() const { return position_[0]; }
  float GetPositionY() const { return position_[1]; }
  float GetPositionZ() const { return position_[2]; }
  float GetTargetX() const { return target_[0]; }
  float GetTargetY() const { return target_[1]; }
  float GetTargetZ() const { return target_[2]; }
  float GetZoomFactor() const { return zoomFactor_; }

  Point GetPosition() const {
    return Point(position_[0], position_[1], position_[2]);
  }

  S21Matrix GetViewMatrix() const;
  S21Matrix GetProjectionMatrix(float aspect) const;

  float GetLeft() const { return -5.0f * zoomFactor_; }
  float GetRight() const { return 5.0f * zoomFactor_; }
  float GetBottom() const { return -5.0f * zoomFactor_; }
  float GetTop() const { return 5.0f * zoomFactor_; }
  float GetNear() const { return nearPlane_; }
  float GetFar() const { return farPlane_; }

  void Rotate(float deltaX, float deltaY);
  void Zoom(float delta);
  void Pan(float deltaX, float deltaY);

 private:
  float position_[3];
  float target_[3];
  float up_[3];
  Settings::ProjectionType projectionType_ =
      Settings::ProjectionType::Orthographic;
  float zoomFactor_ = 1.0f;
  float nearPlane_ = 0.1f;
  float farPlane_ = 100.0f;
};

}  // namespace s21

#endif  // S21_CAMERA_H
