#include <gtest/gtest.h>

#include "matrix/s21_matrix_oop.h"

TEST(MatrixTest, DefaultConstructor) {
  S21Matrix m;
  EXPECT_EQ(m.GetRows(), 0);
  EXPECT_EQ(m.GetCols(), 0);
}

TEST(MatrixTest, ParameterConstructor) {
  S21Matrix m(3, 4);
  EXPECT_EQ(m.GetRows(), 3);
  EXPECT_EQ(m.GetCols(), 4);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 4; ++j) EXPECT_DOUBLE_EQ(m(i, j), 0.0);
}

TEST(MatrixTest, ParameterConstructorInvalid) {
  EXPECT_THROW(S21Matrix(-1, 3), std::invalid_argument);
  EXPECT_THROW(S21Matrix(3, -1), std::invalid_argument);
  EXPECT_THROW(S21Matrix(0, 3), std::invalid_argument);
}

TEST(MatrixTest, CopyConstructor) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  m1(0, 1) = 2;
  m1(1, 0) = 3;
  m1(1, 1) = 4;
  S21Matrix m2(m1);
  EXPECT_EQ(m2.GetRows(), 2);
  EXPECT_EQ(m2.GetCols(), 2);
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j) EXPECT_DOUBLE_EQ(m2(i, j), m1(i, j));
}

TEST(MatrixTest, MoveConstructor) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  m1(0, 1) = 2;
  S21Matrix m2(std::move(m1));
  EXPECT_EQ(m2.GetRows(), 2);
  EXPECT_EQ(m2.GetCols(), 2);
  EXPECT_DOUBLE_EQ(m2(0, 0), 1);
  EXPECT_DOUBLE_EQ(m2(0, 1), 2);
  EXPECT_EQ(m1.GetRows(), 0);
  EXPECT_EQ(m1.GetCols(), 0);
}

TEST(MatrixTest, CopyAssignment) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  m1(0, 1) = 2;
  S21Matrix m2(1, 1);
  m2 = m1;
  EXPECT_EQ(m2.GetRows(), 2);
  EXPECT_EQ(m2.GetCols(), 2);
  EXPECT_DOUBLE_EQ(m2(0, 0), 1);
  EXPECT_DOUBLE_EQ(m2(0, 1), 2);
}

TEST(MatrixTest, MoveAssignment) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  S21Matrix m2;
  m2 = std::move(m1);
  EXPECT_EQ(m2.GetRows(), 2);
  EXPECT_DOUBLE_EQ(m2(0, 0), 1);
  EXPECT_EQ(m1.GetRows(), 0);
}

TEST(MatrixTest, EqMatrix) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  m1(0, 1) = 2;
  S21Matrix m2(2, 2);
  m2(0, 0) = 1;
  m2(0, 1) = 2;
  EXPECT_TRUE(m1.EqMatrix(m2));
  EXPECT_TRUE(m1 == m2);
  m2(1, 1) = 1;
  EXPECT_FALSE(m1.EqMatrix(m2));
  EXPECT_TRUE(m1 != m2);
}

TEST(MatrixTest, SumMatrix) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  m1(0, 1) = 2;
  m1(1, 0) = 3;
  m1(1, 1) = 4;
  S21Matrix m2(2, 2);
  m2(0, 0) = 5;
  m2(0, 1) = 6;
  m2(1, 0) = 7;
  m2(1, 1) = 8;
  m1.SumMatrix(m2);
  EXPECT_DOUBLE_EQ(m1(0, 0), 6);
  EXPECT_DOUBLE_EQ(m1(0, 1), 8);
  EXPECT_DOUBLE_EQ(m1(1, 0), 10);
  EXPECT_DOUBLE_EQ(m1(1, 1), 12);
}

TEST(MatrixTest, SubMatrix) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 5;
  m1(0, 1) = 6;
  m1(1, 0) = 7;
  m1(1, 1) = 8;
  S21Matrix m2(2, 2);
  m2(0, 0) = 1;
  m2(0, 1) = 2;
  m2(1, 0) = 3;
  m2(1, 1) = 4;
  m1.SubMatrix(m2);
  EXPECT_DOUBLE_EQ(m1(0, 0), 4);
  EXPECT_DOUBLE_EQ(m1(0, 1), 4);
  EXPECT_DOUBLE_EQ(m1(1, 0), 4);
  EXPECT_DOUBLE_EQ(m1(1, 1), 4);
}

TEST(MatrixTest, MulNumber) {
  S21Matrix m(2, 2);
  m(0, 0) = 1;
  m(0, 1) = 2;
  m(1, 0) = 3;
  m(1, 1) = 4;
  m.MulNumber(2.0);
  EXPECT_DOUBLE_EQ(m(0, 0), 2);
  EXPECT_DOUBLE_EQ(m(0, 1), 4);
  EXPECT_DOUBLE_EQ(m(1, 0), 6);
  EXPECT_DOUBLE_EQ(m(1, 1), 8);
}

TEST(MatrixTest, MulMatrix) {
  S21Matrix m1(2, 3);
  m1(0, 0) = 1;
  m1(0, 1) = 2;
  m1(0, 2) = 3;
  m1(1, 0) = 4;
  m1(1, 1) = 5;
  m1(1, 2) = 6;
  S21Matrix m2(3, 2);
  m2(0, 0) = 7;
  m2(0, 1) = 8;
  m2(1, 0) = 9;
  m2(1, 1) = 10;
  m2(2, 0) = 11;
  m2(2, 1) = 12;
  m1.MulMatrix(m2);
  EXPECT_EQ(m1.GetRows(), 2);
  EXPECT_EQ(m1.GetCols(), 2);
  EXPECT_DOUBLE_EQ(m1(0, 0), 58);
  EXPECT_DOUBLE_EQ(m1(0, 1), 64);
  EXPECT_DOUBLE_EQ(m1(1, 0), 139);
  EXPECT_DOUBLE_EQ(m1(1, 1), 154);
}

TEST(MatrixTest, Transpose) {
  S21Matrix m(2, 3);
  m(0, 0) = 1;
  m(0, 1) = 2;
  m(0, 2) = 3;
  m(1, 0) = 4;
  m(1, 1) = 5;
  m(1, 2) = 6;
  S21Matrix t = m.Transpose();
  EXPECT_EQ(t.GetRows(), 3);
  EXPECT_EQ(t.GetCols(), 2);
  EXPECT_DOUBLE_EQ(t(0, 0), 1);
  EXPECT_DOUBLE_EQ(t(0, 1), 4);
  EXPECT_DOUBLE_EQ(t(1, 0), 2);
  EXPECT_DOUBLE_EQ(t(1, 1), 5);
  EXPECT_DOUBLE_EQ(t(2, 0), 3);
  EXPECT_DOUBLE_EQ(t(2, 1), 6);
}

TEST(MatrixTest, Determinant) {
  S21Matrix m1(1, 1);
  m1(0, 0) = 5;
  EXPECT_DOUBLE_EQ(m1.Determinant(), 5);

  S21Matrix m2(2, 2);
  m2(0, 0) = 1;
  m2(0, 1) = 2;
  m2(1, 0) = 3;
  m2(1, 1) = 4;
  EXPECT_DOUBLE_EQ(m2.Determinant(), -2);

  S21Matrix m3(3, 3);
  m3(0, 0) = 1;
  m3(0, 1) = 2;
  m3(0, 2) = 3;
  m3(1, 0) = 0;
  m3(1, 1) = 1;
  m3(1, 2) = 4;
  m3(2, 0) = 5;
  m3(2, 1) = 6;
  m3(2, 2) = 0;
  EXPECT_DOUBLE_EQ(m3.Determinant(), 1);
}

TEST(MatrixTest, CalcComplements) {
  S21Matrix m(3, 3);
  m(0, 0) = 1;
  m(0, 1) = 2;
  m(0, 2) = 3;
  m(1, 0) = 0;
  m(1, 1) = 1;
  m(1, 2) = 4;
  m(2, 0) = 5;
  m(2, 1) = 6;
  m(2, 2) = 0;
  S21Matrix comp = m.CalcComplements();
  EXPECT_DOUBLE_EQ(comp(0, 0), (1 * 0 - 4 * 6));
  EXPECT_DOUBLE_EQ(comp(0, 1), -(0 * 0 - 4 * 5));
  EXPECT_DOUBLE_EQ(comp(0, 2), (0 * 6 - 1 * 5));
}

TEST(MatrixTest, InverseMatrix) {
  S21Matrix m(2, 2);
  m(0, 0) = 1;
  m(0, 1) = 2;
  m(1, 0) = 3;
  m(1, 1) = 4;
  S21Matrix inv = m.InverseMatrix();
  double det = -2;
  EXPECT_DOUBLE_EQ(inv(0, 0), 4 / det);
  EXPECT_DOUBLE_EQ(inv(0, 1), -2 / det);
  EXPECT_DOUBLE_EQ(inv(1, 0), -3 / det);
  EXPECT_DOUBLE_EQ(inv(1, 1), 1 / det);

  S21Matrix m2(1, 1);
  m2(0, 0) = 5;
  inv = m2.InverseMatrix();
  EXPECT_DOUBLE_EQ(inv(0, 0), 0.2);
}

TEST(MatrixTest, OperatorParentheses) {
  S21Matrix m(2, 2);
  m(0, 0) = 1;
  EXPECT_DOUBLE_EQ(m(0, 0), 1);
  const S21Matrix& cm = m;
  EXPECT_DOUBLE_EQ(cm(0, 0), 1);
  EXPECT_THROW(m(2, 0), std::out_of_range);
  EXPECT_THROW(m(0, 2), std::out_of_range);
}

TEST(MatrixTest, OperatorAdd) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  S21Matrix m2(2, 2);
  m2(0, 0) = 2;
  S21Matrix m3 = m1 + m2;
  EXPECT_DOUBLE_EQ(m3(0, 0), 3);
}

TEST(MatrixTest, OperatorSub) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 5;
  S21Matrix m2(2, 2);
  m2(0, 0) = 2;
  S21Matrix m3 = m1 - m2;
  EXPECT_DOUBLE_EQ(m3(0, 0), 3);
}

TEST(MatrixTest, OperatorMulMatrix) {
  S21Matrix m1(2, 3);
  m1(0, 0) = 1;
  S21Matrix m2(3, 2);
  m2(0, 0) = 2;
  S21Matrix m3 = m1 * m2;
  EXPECT_EQ(m3.GetRows(), 2);
  EXPECT_EQ(m3.GetCols(), 2);
}

TEST(MatrixTest, OperatorMulNumber) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 3;
  S21Matrix m2 = m1 * 2.0;
  EXPECT_DOUBLE_EQ(m2(0, 0), 6);
  S21Matrix m3 = 2.0 * m1;
  EXPECT_DOUBLE_EQ(m3(0, 0), 6);
}

TEST(MatrixTest, OperatorAddAssign) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 1;
  S21Matrix m2(2, 2);
  m2(0, 0) = 2;
  m1 += m2;
  EXPECT_DOUBLE_EQ(m1(0, 0), 3);
}

TEST(MatrixTest, OperatorSubAssign) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 5;
  S21Matrix m2(2, 2);
  m2(0, 0) = 2;
  m1 -= m2;
  EXPECT_DOUBLE_EQ(m1(0, 0), 3);
}

TEST(MatrixTest, OperatorMulAssignMatrix) {
  S21Matrix m1(2, 3);
  m1(0, 0) = 1;
  S21Matrix m2(3, 2);
  m2(0, 0) = 2;
  m1 *= m2;
  EXPECT_EQ(m1.GetRows(), 2);
  EXPECT_EQ(m1.GetCols(), 2);
}

TEST(MatrixTest, OperatorMulAssignNumber) {
  S21Matrix m1(2, 2);
  m1(0, 0) = 3;
  m1 *= 2.0;
  EXPECT_DOUBLE_EQ(m1(0, 0), 6);
}

TEST(MatrixTest, Resize) {
  S21Matrix m(2, 2);
  m(0, 0) = 1;
  m(0, 1) = 2;
  m(1, 0) = 3;
  m(1, 1) = 4;
  m.SetRows(3);
  EXPECT_EQ(m.GetRows(), 3);
  EXPECT_EQ(m.GetCols(), 2);
  EXPECT_DOUBLE_EQ(m(0, 0), 1);
  EXPECT_DOUBLE_EQ(m(2, 1), 0.0);
  m.SetCols(3);
  EXPECT_EQ(m.GetCols(), 3);
  EXPECT_DOUBLE_EQ(m(2, 2), 0.0);
}

TEST(MatrixTest, ExceptionSumDifferentSizes) {
  S21Matrix m1(2, 2);
  S21Matrix m2(3, 3);
  EXPECT_THROW(m1.SumMatrix(m2), std::logic_error);
}

TEST(MatrixTest, ExceptionMulInvalidDimensions) {
  S21Matrix m1(2, 3);
  S21Matrix m2(2, 2);
  EXPECT_THROW(m1.MulMatrix(m2), std::logic_error);
}

TEST(MatrixTest, ExceptionDeterminantNonSquare) {
  S21Matrix m(2, 3);
  EXPECT_THROW(m.Determinant(), std::logic_error);
}

TEST(MatrixTest, ExceptionInverseSingular) {
  S21Matrix m(2, 2);
  m(0, 0) = 1;
  m(0, 1) = 2;
  m(1, 0) = 2;
  m(1, 1) = 4;
  EXPECT_THROW(m.InverseMatrix(), std::logic_error);
}
