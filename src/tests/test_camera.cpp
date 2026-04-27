#include <gtest/gtest.h>

#include "scene/camera.h"
#include "settings/settings.h"

using namespace s21;

class CameraTest : public ::testing::Test {
 protected:
  Camera cam;
};

TEST_F(CameraTest, DefaultConstructor) {
  S21Matrix view = cam.GetViewMatrix();
  EXPECT_EQ(view.GetRows(), 4);
  EXPECT_EQ(view.GetCols(), 4);
}

TEST_F(CameraTest, SetPosition) {
  cam.SetPosition(1, 2, 3);
  S21Matrix view = cam.GetViewMatrix();
  Camera cam2;
  S21Matrix view_default = cam2.GetViewMatrix();
  EXPECT_NE(view(0, 3), view_default(0, 3));
}

TEST_F(CameraTest, SetTarget) {
  cam.SetTarget(5, 5, 5);
  S21Matrix view = cam.GetViewMatrix();
  Camera cam2;
  S21Matrix view_default = cam2.GetViewMatrix();
  EXPECT_NE(view(2, 0), view_default(2, 0));
}

TEST_F(CameraTest, SetUp) {
  cam.SetUp(0, 0, 1);
  S21Matrix view = cam.GetViewMatrix();
  Camera cam2;
  S21Matrix view_default = cam2.GetViewMatrix();
  bool equal = true;
  for (int i = 0; i < 4 && equal; ++i) {
    for (int j = 0; j < 4 && equal; ++j) {
      if (view(i, j) != view_default(i, j)) equal = false;
    }
  }
  EXPECT_FALSE(equal);
}

TEST_F(CameraTest, ProjectionTypeOrthographic) {
  cam.SetProjectionType(Settings::ProjectionType::Orthographic);
  S21Matrix proj = cam.GetProjectionMatrix(1.0f);
  EXPECT_EQ(proj(3, 0), 0.0);
  EXPECT_EQ(proj(3, 1), 0.0);
  EXPECT_EQ(proj(3, 2), 0.0);
  EXPECT_EQ(proj(3, 3), 1.0);
}

TEST_F(CameraTest, ProjectionTypePerspective) {
  cam.SetProjectionType(Settings::ProjectionType::Perspective);
  S21Matrix proj = cam.GetProjectionMatrix(1.0f);
  EXPECT_EQ(proj(3, 2), -1.0);
}

TEST_F(CameraTest, GetProjectionMatrixAspectPerspective) {
  cam.SetProjectionType(Settings::ProjectionType::Perspective);
  S21Matrix proj1 = cam.GetProjectionMatrix(1.0f);
  S21Matrix proj2 = cam.GetProjectionMatrix(2.0f);
  EXPECT_NE(proj1(0, 0), proj2(0, 0));
}

TEST_F(CameraTest, OrthographicParameters) {
  EXPECT_FLOAT_EQ(cam.GetLeft(), -5.0f);
  EXPECT_FLOAT_EQ(cam.GetRight(), 5.0f);
  EXPECT_FLOAT_EQ(cam.GetBottom(), -5.0f);
  EXPECT_FLOAT_EQ(cam.GetTop(), 5.0f);
  EXPECT_FLOAT_EQ(cam.GetNear(), 0.1f);
  EXPECT_FLOAT_EQ(cam.GetFar(), 100.0f);
}

TEST_F(CameraTest, Zoom) {
  cam.SetPosition(0, 0, 10);
  cam.SetTarget(0, 0, 0);
  float leftBefore = cam.GetLeft();
  float zoomBefore = cam.GetZoomFactor();
  cam.Zoom(1.0f);  // приближение
  float leftAfter = cam.GetLeft();
  float zoomAfter = cam.GetZoomFactor();
  // При приближении поле обзора сужается: left становится больше (ближе к нулю)
  EXPECT_GT(leftAfter, leftBefore);
  EXPECT_LT(zoomAfter, zoomBefore);
}

TEST_F(CameraTest, Rotate) {
  cam.SetPosition(0, 0, 10);
  cam.SetTarget(0, 0, 0);
  float oldX = cam.GetPositionX();
  float oldY = cam.GetPositionY();
  float oldZ = cam.GetPositionZ();
  cam.Rotate(10.0f, 5.0f);
  EXPECT_NE(cam.GetPositionX(), oldX);
  EXPECT_NE(cam.GetPositionY(), oldY);
  EXPECT_NE(cam.GetPositionZ(), oldZ);
}

TEST_F(CameraTest, Pan) {
  cam.SetPosition(0, 0, 10);
  cam.SetTarget(0, 0, 0);
  float oldX = cam.GetPositionX();
  float oldY = cam.GetPositionY();
  float oldTargetX = cam.GetTargetX();
  float oldTargetY = cam.GetTargetY();
  cam.Pan(0.1f, 0.1f);
  EXPECT_NE(cam.GetPositionX(), oldX);
  EXPECT_NE(cam.GetPositionY(), oldY);
  EXPECT_NE(cam.GetTargetX(), oldTargetX);
  EXPECT_NE(cam.GetTargetY(), oldTargetY);
}

TEST_F(CameraTest, SetZoomFactor) {
  cam.SetZoomFactor(2.0f);
  EXPECT_FLOAT_EQ(cam.GetZoomFactor(), 2.0f);
  cam.SetZoomFactor(0.00005f);
  EXPECT_FLOAT_EQ(cam.GetZoomFactor(), 0.0001f);
  cam.SetZoomFactor(20000.0f);
  EXPECT_FLOAT_EQ(cam.GetZoomFactor(), 10000.0f);
}

TEST_F(CameraTest, SetNearFarPlanes) {
  cam.SetNearPlane(0.5f);
  cam.SetFarPlane(500.0f);
  EXPECT_FLOAT_EQ(cam.GetNear(), 0.5f);
  EXPECT_FLOAT_EQ(cam.GetFar(), 500.0f);
}
