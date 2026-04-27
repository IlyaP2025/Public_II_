#include <gtest/gtest.h>

#include "common/point.h"
#include "common/transform.h"

using namespace s21;

// Тест 1: Проверка матрицы перемещения
TEST(TransformTest, Translation) {
  Transform t;
  t.SetPosition(Point(2.0f, 3.0f, 4.0f));
  S21Matrix m = t.GetModelMatrix();

  EXPECT_DOUBLE_EQ(m(0, 3), 2.0);
  EXPECT_DOUBLE_EQ(m(1, 3), 3.0);
  EXPECT_DOUBLE_EQ(m(2, 3), 4.0);
  EXPECT_DOUBLE_EQ(m(3, 3), 1.0);
}

// Тест 2: Проверка масштабирования
TEST(TransformTest, Scale) {
  Transform t;
  t.SetScale(Point(2.0f, 3.0f, 4.0f));
  S21Matrix m = t.GetModelMatrix();

  EXPECT_DOUBLE_EQ(m(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(m(1, 1), 3.0);
  EXPECT_DOUBLE_EQ(m(2, 2), 4.0);
  EXPECT_DOUBLE_EQ(m(3, 3), 1.0);
}

// Тест 3: Поворот на 90° вокруг X
TEST(TransformTest, RotationX) {
  Transform t;
  t.SetRotation(Point(90.0f, 0.0f, 0.0f));
  S21Matrix m = t.GetModelMatrix();

  // Ожидаем: y' = y*cos - z*sin = -z, z' = y*sin + z*cos = y
  EXPECT_NEAR(m(1, 1), 0.0, 1e-6);
  EXPECT_NEAR(m(1, 2), -1.0, 1e-6);
  EXPECT_NEAR(m(2, 1), 1.0, 1e-6);
  EXPECT_NEAR(m(2, 2), 0.0, 1e-6);
}

// Тест 4: Комбинированная трансформация
TEST(TransformTest, Combined) {
  Transform t;
  t.SetPosition(Point(1.0f, 2.0f, 3.0f));
  t.SetRotation(Point(90.0f, 0.0f, 0.0f));
  t.SetScale(Point(2.0f, 2.0f, 2.0f));

  S21Matrix m = t.GetModelMatrix();

  // Проверяем перемещение
  EXPECT_DOUBLE_EQ(m(0, 3), 1.0);
  EXPECT_DOUBLE_EQ(m(1, 3), 2.0);
  EXPECT_DOUBLE_EQ(m(2, 3), 3.0);
  EXPECT_DOUBLE_EQ(m(3, 3), 1.0);

  // Проверяем масштабирование и поворот: элемент (1,2) должен быть -2
  EXPECT_NEAR(m(1, 2), -2.0, 1e-6);
  EXPECT_NEAR(m(2, 1), 2.0, 1e-6);
}