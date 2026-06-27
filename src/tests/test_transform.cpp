#include <gtest/gtest.h>
#include <numbers>

#include "common/point.h"
#include "common/transform.h"

using namespace s21;

TEST(TransformTest, Translation) {
  Transform t;
  t.SetPosition(Point(2.0f, 3.0f, 4.0f));
  S21Matrix m = t.GetModelMatrix();

  EXPECT_DOUBLE_EQ(m(0, 3), 2.0);
  EXPECT_DOUBLE_EQ(m(1, 3), 3.0);
  EXPECT_DOUBLE_EQ(m(2, 3), 4.0);
  EXPECT_DOUBLE_EQ(m(3, 3), 1.0);
}

TEST(TransformTest, Scale) {
  Transform t;
  t.SetScale(Point(2.0f, 3.0f, 4.0f));
  S21Matrix m = t.GetModelMatrix();

  EXPECT_DOUBLE_EQ(m(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(m(1, 1), 3.0);
  EXPECT_DOUBLE_EQ(m(2, 2), 4.0);
  EXPECT_DOUBLE_EQ(m(3, 3), 1.0);
}

TEST(TransformTest, RotationX) {
  Transform t;
  t.SetRotation(Point(90.0f, 0.0f, 0.0f));
  S21Matrix m = t.GetModelMatrix();

  EXPECT_NEAR(m(1, 1), 0.0, 1e-6);
  EXPECT_NEAR(m(1, 2), -1.0, 1e-6);
  EXPECT_NEAR(m(2, 1), 1.0, 1e-6);
  EXPECT_NEAR(m(2, 2), 0.0, 1e-6);
}

TEST(TransformTest, Combined) {
  Transform t;
  t.SetPosition(Point(1.0f, 2.0f, 3.0f));
  t.SetRotation(Point(90.0f, 0.0f, 0.0f));
  t.SetScale(Point(2.0f, 2.0f, 2.0f));

  S21Matrix m = t.GetModelMatrix();

  EXPECT_DOUBLE_EQ(m(0, 3), 1.0);
  EXPECT_DOUBLE_EQ(m(1, 3), 2.0);
  EXPECT_DOUBLE_EQ(m(2, 3), 3.0);
  EXPECT_DOUBLE_EQ(m(3, 3), 1.0);

  EXPECT_NEAR(m(1, 2), -2.0, 1e-6);
  EXPECT_NEAR(m(2, 1), 2.0, 1e-6);
}

TEST(TransformTest, TransformPointIdentity) {
  S21Matrix identity(4, 4);
  identity(0,0)=1; identity(1,1)=1; identity(2,2)=1; identity(3,3)=1;
  Point p(1, 2, 3);
  Point res = TransformPoint(identity, p);
  EXPECT_FLOAT_EQ(res.x, 1);
  EXPECT_FLOAT_EQ(res.y, 2);
  EXPECT_FLOAT_EQ(res.z, 3);
}

TEST(TransformTest, TransformPointTranslation) {
  S21Matrix translate(4, 4);
  translate(0,0)=1; translate(1,1)=1; translate(2,2)=1; translate(3,3)=1;
  translate(0,3)=5; translate(1,3)=6; translate(2,3)=7;
  Point p(1, 2, 3);
  Point res = TransformPoint(translate, p);
  EXPECT_FLOAT_EQ(res.x, 6);
  EXPECT_FLOAT_EQ(res.y, 8);
  EXPECT_FLOAT_EQ(res.z, 10);
}

TEST(TransformTest, TransformDirectionIdentity) {
  S21Matrix identity(4, 4);
  identity(0,0)=1; identity(1,1)=1; identity(2,2)=1; identity(3,3)=1;
  Point dir(1, 2, 3);
  Point res = TransformDirection(identity, dir);
  EXPECT_FLOAT_EQ(res.x, 1);
  EXPECT_FLOAT_EQ(res.y, 2);
  EXPECT_FLOAT_EQ(res.z, 3);
}

TEST(TransformTest, TransformDirectionRotation) {
  double a = std::numbers::pi / 2.0;
  S21Matrix rot(4, 4);
  rot(0,0)=1;
  rot(1,1)=std::cos(a); rot(1,2)=-std::sin(a);
  rot(2,1)=std::sin(a); rot(2,2)=std::cos(a);
  rot(3,3)=1.0;
  Point dir(0, 1, 0);
  Point res = TransformDirection(rot, dir);
  EXPECT_NEAR(res.x, 0, 1e-6);
  EXPECT_NEAR(res.y, 0, 1e-6);
  EXPECT_NEAR(res.z, 1, 1e-6);
}
