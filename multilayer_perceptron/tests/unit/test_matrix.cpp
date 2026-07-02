#include <gtest/gtest.h>
#include "matrix/s21_matrix_oop.h"

// ==================== ТЕСТЫ КОНСТРУКТОРОВ ====================

TEST(ConstructorTest, DefaultConstructor) {
  S21Matrix matrix;
  EXPECT_EQ(matrix.get_rows(), 3);
  EXPECT_EQ(matrix.get_cols(), 3);

  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      EXPECT_DOUBLE_EQ(matrix(i, j), 0.0);
    }
  }
}

TEST(ConstructorTest, ParameterizedConstructor) {
  S21Matrix matrix(2, 3);
  EXPECT_EQ(matrix.get_rows(), 2);
  EXPECT_EQ(matrix.get_cols(), 3);

  EXPECT_THROW(S21Matrix matrix2(-1, 5), std::invalid_argument);
  EXPECT_THROW(S21Matrix matrix3(0, 5), std::invalid_argument);
}

TEST(ConstructorTest, CopyConstructor) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;
  matrix1(0, 1) = 2.0;
  matrix1(1, 0) = 3.0;
  matrix1(1, 1) = 4.0;

  S21Matrix matrix2(matrix1);
  EXPECT_EQ(matrix2.get_rows(), 2);
  EXPECT_EQ(matrix2.get_cols(), 2);
  EXPECT_DOUBLE_EQ(matrix2(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix2(0, 1), 2.0);
}

TEST(ConstructorTest, MoveConstructor) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;

  S21Matrix matrix2(std::move(matrix1));
  EXPECT_EQ(matrix2.get_rows(), 2);
  EXPECT_EQ(matrix2.get_cols(), 2);
  EXPECT_DOUBLE_EQ(matrix2(0, 0), 1.0);
  EXPECT_EQ(matrix1.get_rows(), 0);
  EXPECT_EQ(matrix1.get_cols(), 0);
}

// ==================== ТЕСТЫ ОПЕРАТОРОВ ПРИСВАИВАНИЯ ====================

TEST(AssignmentTest, CopyAssignment) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;

  S21Matrix matrix2;
  matrix2 = matrix1;

  EXPECT_EQ(matrix2.get_rows(), 2);
  EXPECT_EQ(matrix2.get_cols(), 2);
  EXPECT_DOUBLE_EQ(matrix2(0, 0), 1.0);

  // Self-assignment
  matrix2 = matrix2;
  EXPECT_DOUBLE_EQ(matrix2(0, 0), 1.0);
}

TEST(AssignmentTest, MoveAssignment) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;

  S21Matrix matrix2;
  matrix2 = std::move(matrix1);

  EXPECT_EQ(matrix2.get_rows(), 2);
  EXPECT_EQ(matrix2.get_cols(), 2);
  EXPECT_DOUBLE_EQ(matrix2(0, 0), 1.0);
  EXPECT_EQ(matrix1.get_rows(), 0);
  EXPECT_EQ(matrix1.get_cols(), 0);
}

// ==================== ТЕСТЫ ACCESSOR/MUTATOR ====================

TEST(AccessorTest, GetSetRows) {
  S21Matrix matrix(2, 3);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(0, 2) = 3.0;
  matrix(1, 0) = 4.0;
  matrix(1, 1) = 5.0;
  matrix(1, 2) = 6.0;

  EXPECT_EQ(matrix.get_rows(), 2);
  EXPECT_EQ(matrix.get_cols(), 3);

  matrix.set_rows(3);
  EXPECT_EQ(matrix.get_rows(), 3);
  EXPECT_EQ(matrix.get_cols(), 3);
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(matrix(2, 0), 0.0);  // Новая строка должна быть нулевой

  EXPECT_THROW(matrix.set_rows(0), std::invalid_argument);
  EXPECT_THROW(matrix.set_rows(-1), std::invalid_argument);
}

TEST(AccessorTest, GetSetCols) {
  S21Matrix matrix(2, 2);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(1, 0) = 3.0;
  matrix(1, 1) = 4.0;

  matrix.set_cols(3);
  EXPECT_EQ(matrix.get_rows(), 2);
  EXPECT_EQ(matrix.get_cols(), 3);
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(matrix(0, 2), 0.0);  // Новый столбец должен быть нулевым

  EXPECT_THROW(matrix.set_cols(0), std::invalid_argument);
  EXPECT_THROW(matrix.set_cols(-1), std::invalid_argument);
}

// ==================== ТЕСТЫ ОСНОВНЫХ МЕТОДОВ ====================

TEST(EqMatrixTest, Basic) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.00000001;
  matrix1(0, 1) = 2.0;
  matrix1(1, 0) = 3.0;
  matrix1(1, 1) = 4.0;

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 1.0;
  matrix2(0, 1) = 2.0;
  matrix2(1, 0) = 3.0;
  matrix2(1, 1) = 4.0;

  EXPECT_TRUE(matrix1.EqMatrix(matrix2));

  S21Matrix matrix3(3, 2);
  EXPECT_FALSE(matrix1.EqMatrix(matrix3));
}

TEST(SumMatrixTest, Basic) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;
  matrix1(0, 1) = 2.0;
  matrix1(1, 0) = 3.0;
  matrix1(1, 1) = 4.0;

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 5.0;
  matrix2(0, 1) = 6.0;
  matrix2(1, 0) = 7.0;
  matrix2(1, 1) = 8.0;

  matrix1.SumMatrix(matrix2);

  EXPECT_DOUBLE_EQ(matrix1(0, 0), 6.0);
  EXPECT_DOUBLE_EQ(matrix1(0, 1), 8.0);
  EXPECT_DOUBLE_EQ(matrix1(1, 0), 10.0);
  EXPECT_DOUBLE_EQ(matrix1(1, 1), 12.0);

  S21Matrix matrix3(3, 3);
  EXPECT_THROW(matrix1.SumMatrix(matrix3), std::invalid_argument);
}

TEST(SubMatrixTest, Basic) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 5.0;
  matrix1(0, 1) = 6.0;
  matrix1(1, 0) = 7.0;
  matrix1(1, 1) = 8.0;

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 1.0;
  matrix2(0, 1) = 2.0;
  matrix2(1, 0) = 3.0;
  matrix2(1, 1) = 4.0;

  matrix1.SubMatrix(matrix2);

  EXPECT_DOUBLE_EQ(matrix1(0, 0), 4.0);
  EXPECT_DOUBLE_EQ(matrix1(0, 1), 4.0);
  EXPECT_DOUBLE_EQ(matrix1(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(matrix1(1, 1), 4.0);
}

TEST(MulNumberTest, Basic) {
  S21Matrix matrix(2, 2);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(1, 0) = 3.0;
  matrix(1, 1) = 4.0;

  matrix.MulNumber(2.5);

  EXPECT_DOUBLE_EQ(matrix(0, 0), 2.5);
  EXPECT_DOUBLE_EQ(matrix(0, 1), 5.0);
  EXPECT_DOUBLE_EQ(matrix(1, 0), 7.5);
  EXPECT_DOUBLE_EQ(matrix(1, 1), 10.0);
}

TEST(MulMatrixTest, Basic) {
  S21Matrix matrix1(2, 3);
  matrix1(0, 0) = 1.0;
  matrix1(0, 1) = 2.0;
  matrix1(0, 2) = 3.0;
  matrix1(1, 0) = 4.0;
  matrix1(1, 1) = 5.0;
  matrix1(1, 2) = 6.0;

  S21Matrix matrix2(3, 2);
  matrix2(0, 0) = 7.0;
  matrix2(0, 1) = 8.0;
  matrix2(1, 0) = 9.0;
  matrix2(1, 1) = 10.0;
  matrix2(2, 0) = 11.0;
  matrix2(2, 1) = 12.0;

  matrix1.MulMatrix(matrix2);

  EXPECT_EQ(matrix1.get_rows(), 2);
  EXPECT_EQ(matrix1.get_cols(), 2);

  EXPECT_DOUBLE_EQ(matrix1(0, 0), 58.0);
  EXPECT_DOUBLE_EQ(matrix1(0, 1), 64.0);
  EXPECT_DOUBLE_EQ(matrix1(1, 0), 139.0);
  EXPECT_DOUBLE_EQ(matrix1(1, 1), 154.0);

  // Создаем матрицу несовместимого размера для умножения
  S21Matrix matrix3(4, 4);  // Несовместимый размер - 4x4
  EXPECT_THROW(matrix1.MulMatrix(matrix3), std::invalid_argument);
}

TEST(TransposeTest, Basic) {
  S21Matrix matrix(2, 3);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(0, 2) = 3.0;
  matrix(1, 0) = 4.0;
  matrix(1, 1) = 5.0;
  matrix(1, 2) = 6.0;

  S21Matrix transposed = matrix.Transpose();

  EXPECT_EQ(transposed.get_rows(), 3);
  EXPECT_EQ(transposed.get_cols(), 2);

  EXPECT_DOUBLE_EQ(transposed(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(transposed(0, 1), 4.0);
  EXPECT_DOUBLE_EQ(transposed(1, 0), 2.0);
  EXPECT_DOUBLE_EQ(transposed(1, 1), 5.0);
  EXPECT_DOUBLE_EQ(transposed(2, 0), 3.0);
  EXPECT_DOUBLE_EQ(transposed(2, 1), 6.0);
}

TEST(CalcComplementsTest, Basic) {
  S21Matrix matrix(3, 3);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(0, 2) = 3.0;
  matrix(1, 0) = 0.0;
  matrix(1, 1) = 4.0;
  matrix(1, 2) = 2.0;
  matrix(2, 0) = 5.0;
  matrix(2, 1) = 2.0;
  matrix(2, 2) = 1.0;

  S21Matrix complements = matrix.CalcComplements();

  EXPECT_DOUBLE_EQ(complements(0, 0), 0.0);
  EXPECT_DOUBLE_EQ(complements(0, 1), 10.0);
  EXPECT_DOUBLE_EQ(complements(0, 2), -20.0);
  EXPECT_DOUBLE_EQ(complements(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(complements(1, 1), -14.0);
  EXPECT_DOUBLE_EQ(complements(1, 2), 8.0);

  EXPECT_THROW(S21Matrix(1, 1).CalcComplements(), std::logic_error);
  EXPECT_THROW(S21Matrix(2, 3).CalcComplements(), std::logic_error);
}

TEST(DeterminantTest, Basic) {
  S21Matrix matrix1(1, 1);
  matrix1(0, 0) = 5.0;
  EXPECT_DOUBLE_EQ(matrix1.Determinant(), 5.0);

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 1.0;
  matrix2(0, 1) = 2.0;
  matrix2(1, 0) = 3.0;
  matrix2(1, 1) = 4.0;
  EXPECT_DOUBLE_EQ(matrix2.Determinant(), -2.0);

  S21Matrix matrix3(3, 3);
  matrix3(0, 0) = 1.0;
  matrix3(0, 1) = 2.0;
  matrix3(0, 2) = 3.0;
  matrix3(1, 0) = 4.0;
  matrix3(1, 1) = 5.0;
  matrix3(1, 2) = 6.0;
  matrix3(2, 0) = 7.0;
  matrix3(2, 1) = 8.0;
  matrix3(2, 2) = 9.0;
  EXPECT_DOUBLE_EQ(matrix3.Determinant(), 0.0);

  S21Matrix matrix4(2, 3);
  EXPECT_THROW(matrix4.Determinant(), std::logic_error);
}

TEST(InverseMatrixTest, Basic) {
  S21Matrix matrix(3, 3);
  matrix(0, 0) = 2.0;
  matrix(0, 1) = 5.0;
  matrix(0, 2) = 7.0;
  matrix(1, 0) = 6.0;
  matrix(1, 1) = 3.0;
  matrix(1, 2) = 4.0;
  matrix(2, 0) = 5.0;
  matrix(2, 1) = -2.0;
  matrix(2, 2) = -3.0;

  S21Matrix inverse = matrix.InverseMatrix();

  // Проверяем, что A * A^(-1) = I
  S21Matrix identity = matrix * inverse;

  EXPECT_NEAR(identity(0, 0), 1.0, 1e-7);
  EXPECT_NEAR(identity(0, 1), 0.0, 1e-7);
  EXPECT_NEAR(identity(0, 2), 0.0, 1e-7);
  EXPECT_NEAR(identity(1, 0), 0.0, 1e-7);
  EXPECT_NEAR(identity(1, 1), 1.0, 1e-7);
  EXPECT_NEAR(identity(1, 2), 0.0, 1e-7);
  EXPECT_NEAR(identity(2, 0), 0.0, 1e-7);
  EXPECT_NEAR(identity(2, 1), 0.0, 1e-7);
  EXPECT_NEAR(identity(2, 2), 1.0, 1e-7);

  S21Matrix singular(2, 2);
  singular(0, 0) = 1.0;
  singular(0, 1) = 2.0;
  singular(1, 0) = 2.0;
  singular(1, 1) = 4.0;
  EXPECT_THROW(singular.InverseMatrix(), std::logic_error);
}

// ==================== ТЕСТЫ ПЕРЕГРУЖЕННЫХ ОПЕРАТОРОВ ====================

TEST(OperatorTest, Addition) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;
  matrix1(0, 1) = 2.0;
  matrix1(1, 0) = 3.0;
  matrix1(1, 1) = 4.0;

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 5.0;
  matrix2(0, 1) = 6.0;
  matrix2(1, 0) = 7.0;
  matrix2(1, 1) = 8.0;

  S21Matrix sum = matrix1 + matrix2;
  EXPECT_DOUBLE_EQ(sum(0, 0), 6.0);
  EXPECT_DOUBLE_EQ(sum(0, 1), 8.0);
  EXPECT_DOUBLE_EQ(sum(1, 0), 10.0);
  EXPECT_DOUBLE_EQ(sum(1, 1), 12.0);

  matrix1 += matrix2;
  EXPECT_DOUBLE_EQ(matrix1(0, 0), 6.0);
}

TEST(OperatorTest, Subtraction) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 5.0;
  matrix1(0, 1) = 6.0;
  matrix1(1, 0) = 7.0;
  matrix1(1, 1) = 8.0;

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 1.0;
  matrix2(0, 1) = 2.0;
  matrix2(1, 0) = 3.0;
  matrix2(1, 1) = 4.0;

  S21Matrix diff = matrix1 - matrix2;
  EXPECT_DOUBLE_EQ(diff(0, 0), 4.0);
  EXPECT_DOUBLE_EQ(diff(0, 1), 4.0);
  EXPECT_DOUBLE_EQ(diff(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(diff(1, 1), 4.0);

  matrix1 -= matrix2;
  EXPECT_DOUBLE_EQ(matrix1(0, 0), 4.0);
}

TEST(OperatorTest, Multiplication) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;
  matrix1(0, 1) = 2.0;
  matrix1(1, 0) = 3.0;
  matrix1(1, 1) = 4.0;

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 5.0;
  matrix2(0, 1) = 6.0;
  matrix2(1, 0) = 7.0;
  matrix2(1, 1) = 8.0;

  S21Matrix mul = matrix1 * matrix2;
  EXPECT_DOUBLE_EQ(mul(0, 0), 19.0);
  EXPECT_DOUBLE_EQ(mul(0, 1), 22.0);
  EXPECT_DOUBLE_EQ(mul(1, 0), 43.0);
  EXPECT_DOUBLE_EQ(mul(1, 1), 50.0);

  matrix1 *= matrix2;
  EXPECT_DOUBLE_EQ(matrix1(0, 0), 19.0);
}

TEST(OperatorTest, ScalarMultiplication) {
  S21Matrix matrix(2, 2);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(1, 0) = 3.0;
  matrix(1, 1) = 4.0;

  S21Matrix mul1 = matrix * 2.0;
  EXPECT_DOUBLE_EQ(mul1(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(mul1(0, 1), 4.0);

  S21Matrix mul2 = 2.0 * matrix;
  EXPECT_DOUBLE_EQ(mul2(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(mul2(0, 1), 4.0);

  matrix *= 2.0;
  EXPECT_DOUBLE_EQ(matrix(0, 0), 2.0);
}

TEST(OperatorTest, Equality) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;
  matrix1(0, 1) = 2.0;
  matrix1(1, 0) = 3.0;
  matrix1(1, 1) = 4.0;

  S21Matrix matrix2(2, 2);
  matrix2(0, 0) = 1.0;
  matrix2(0, 1) = 2.0;
  matrix2(1, 0) = 3.0;
  matrix2(1, 1) = 4.0;

  EXPECT_TRUE(matrix1 == matrix2);

  matrix2(1, 1) = 5.0;
  EXPECT_FALSE(matrix1 == matrix2);
}

TEST(OperatorTest, Indexing) {
  S21Matrix matrix(2, 2);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(1, 0) = 3.0;
  matrix(1, 1) = 4.0;

  // Проверка записи
  matrix(0, 0) = 10.0;
  EXPECT_DOUBLE_EQ(matrix(0, 0), 10.0);

  // Проверка чтения
  double value = matrix(1, 1);
  EXPECT_DOUBLE_EQ(value, 4.0);

  // Проверка исключений
  EXPECT_THROW(matrix(-1, 0), std::out_of_range);
  EXPECT_THROW(matrix(2, 0), std::out_of_range);
  EXPECT_THROW(matrix(0, -1), std::out_of_range);
  EXPECT_THROW(matrix(0, 2), std::out_of_range);

  // Константная версия
  const S21Matrix const_matrix = matrix;
  EXPECT_DOUBLE_EQ(const_matrix(0, 0), 10.0);
  EXPECT_THROW(const_matrix(-1, 0), std::out_of_range);
}

// ==================== НОВЫЕ ТЕСТЫ ДЛЯ ПОЛНОГО ПОКРЫТИЯ ====================

// Тесты для операторов присваивания с исключениями
TEST(OperatorTest, AdditionAssignmentInvalid) {
  S21Matrix matrix1(2, 2);
  S21Matrix matrix2(3, 3);
  
  EXPECT_THROW(matrix1 += matrix2, std::invalid_argument);
}

TEST(OperatorTest, SubtractionAssignmentInvalid) {
  S21Matrix matrix1(2, 2);
  S21Matrix matrix2(3, 3);
  
  EXPECT_THROW(matrix1 -= matrix2, std::invalid_argument);
}

TEST(OperatorTest, MultiplicationAssignmentInvalid) {
  S21Matrix matrix1(2, 3);
  S21Matrix matrix2(4, 2);  // Несовместимые размеры: 3 != 4
  
  EXPECT_THROW(matrix1 *= matrix2, std::invalid_argument);
}

// Тесты для мутаторов с различными сценариями
TEST(MutatorTest, SetRowsIncreaseDecrease) {
  S21Matrix matrix(3, 3);
  
  // Заполняем матрицу
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      matrix(i, j) = i * 3 + j + 1;
    }
  }
  
  // Уменьшаем количество строк
  matrix.set_rows(2);
  EXPECT_EQ(matrix.get_rows(), 2);
  EXPECT_EQ(matrix.get_cols(), 3);
  
  // Проверяем, что данные сохранились
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(matrix(0, 2), 3.0);
  EXPECT_DOUBLE_EQ(matrix(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(matrix(1, 1), 5.0);
  EXPECT_DOUBLE_EQ(matrix(1, 2), 6.0);
  
  // Увеличиваем количество строк обратно
  matrix.set_rows(3);
  EXPECT_EQ(matrix.get_rows(), 3);
  
  // Проверяем, что старые данные сохранились, а новые строки нулевые
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(matrix(2, 0), 0.0);  // Новая строка должна быть нулевой
}

TEST(MutatorTest, SetColsIncreaseDecrease) {
  S21Matrix matrix(3, 3);
  
  // Заполняем матрицу
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      matrix(i, j) = i * 3 + j + 1;
    }
  }
  
  // Уменьшаем количество столбцов
  matrix.set_cols(2);
  EXPECT_EQ(matrix.get_rows(), 3);
  EXPECT_EQ(matrix.get_cols(), 2);
  
  // Проверяем, что данные сохранились
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(matrix(1, 0), 4.0);
  EXPECT_DOUBLE_EQ(matrix(1, 1), 5.0);
  EXPECT_DOUBLE_EQ(matrix(2, 0), 7.0);
  EXPECT_DOUBLE_EQ(matrix(2, 1), 8.0);
  
  // Увеличиваем количество столбцов обратно
  matrix.set_cols(3);
  EXPECT_EQ(matrix.get_cols(), 3);
  
  // Проверяем, что старые данные сохранились, а новые столбцы нулевые
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(matrix(0, 2), 0.0);  // Новый столбец должен быть нулевым
}

// Тесты для операторов с различными сценариями
TEST(OperatorTest, ScalarMultiplicationAssignment) {
  S21Matrix matrix(2, 2);
  matrix(0, 0) = 2.0;
  matrix(0, 1) = 4.0;
  matrix(1, 0) = 6.0;
  matrix(1, 1) = 8.0;
  
  matrix *= 0.5;
  
  EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(matrix(1, 0), 3.0);
  EXPECT_DOUBLE_EQ(matrix(1, 1), 4.0);
}

// Тесты для конструкторов с граничными случаями
TEST(ConstructorTest, EdgeCases) {
  // Матрица 1x1
  S21Matrix matrix1x1(1, 1);
  matrix1x1(0, 0) = 42.0;
  EXPECT_EQ(matrix1x1.get_rows(), 1);
  EXPECT_EQ(matrix1x1.get_cols(), 1);
  EXPECT_DOUBLE_EQ(matrix1x1(0, 0), 42.0);
  
  // Большая матрица
  S21Matrix large(100, 100);
  EXPECT_EQ(large.get_rows(), 100);
  EXPECT_EQ(large.get_cols(), 100);
  
  // Проверяем, что все элементы инициализированы нулями
  for (int i = 0; i < 10; ++i) {  // Проверяем только часть для скорости
    for (int j = 0; j < 10; ++j) {
      EXPECT_DOUBLE_EQ(large(i, j), 0.0);
    }
  }
}

// Тесты для методов с граничными случаями
TEST(MethodTest, TransposeEdgeCases) {
  // Матрица 1xN
  S21Matrix matrix1x3(1, 3);
  matrix1x3(0, 0) = 1.0;
  matrix1x3(0, 1) = 2.0;
  matrix1x3(0, 2) = 3.0;
  
  S21Matrix transposed = matrix1x3.Transpose();
  EXPECT_EQ(transposed.get_rows(), 3);
  EXPECT_EQ(transposed.get_cols(), 1);
  EXPECT_DOUBLE_EQ(transposed(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(transposed(1, 0), 2.0);
  EXPECT_DOUBLE_EQ(transposed(2, 0), 3.0);
}

TEST(MethodTest, DeterminantEdgeCases) {
  // Матрица с очень маленьким определителем
  S21Matrix matrix(2, 2);
  matrix(0, 0) = 1e-10;
  matrix(0, 1) = 2e-10;
  matrix(1, 0) = 3e-10;
  matrix(1, 1) = 4e-10;
  
  double det = matrix.Determinant();
  EXPECT_NEAR(det, -2e-20, 1e-25);
}

// Тесты для проверки корректности работы с памятью
TEST(MemoryTest, DeepCopy) {
  S21Matrix matrix1(2, 2);
  matrix1(0, 0) = 1.0;
  matrix1(0, 1) = 2.0;
  matrix1(1, 0) = 3.0;
  matrix1(1, 1) = 4.0;
  
  // Создаем копию
  S21Matrix matrix2 = matrix1;
  
  // Меняем оригинал
  matrix1(0, 0) = 100.0;
  
  // Копия не должна измениться
  EXPECT_DOUBLE_EQ(matrix2(0, 0), 1.0);
  EXPECT_DOUBLE_EQ(matrix2(0, 1), 2.0);
  EXPECT_DOUBLE_EQ(matrix2(1, 0), 3.0);
  EXPECT_DOUBLE_EQ(matrix2(1, 1), 4.0);
}

// Тесты для проверки цепочки операций
TEST(OperatorTest, OperatorChaining) {
  S21Matrix A(2, 2);
  A(0, 0) = 1.0; A(0, 1) = 2.0;
  A(1, 0) = 3.0; A(1, 1) = 4.0;
  
  S21Matrix B(2, 2);
  B(0, 0) = 5.0; B(0, 1) = 6.0;
  B(1, 0) = 7.0; B(1, 1) = 8.0;
  
  S21Matrix C(2, 2);
  C(0, 0) = 9.0; C(0, 1) = 10.0;
  C(1, 0) = 11.0; C(1, 1) = 12.0;
  
  // Цепочка операций
  S21Matrix result = A + B - C;
  
  // Проверяем вручную
  S21Matrix expected(2, 2);
  expected(0, 0) = -3.0; expected(0, 1) = -2.0;
  expected(1, 0) = -1.0; expected(1, 1) = 0.0;
  
  EXPECT_TRUE(result == expected);
}

// Тесты для проверки дружественного оператора
TEST(OperatorTest, FriendOperator) {
  S21Matrix matrix(2, 2);
  matrix(0, 0) = 1.0;
  matrix(0, 1) = 2.0;
  matrix(1, 0) = 3.0;
  matrix(1, 1) = 4.0;
  
  // Умножение числа на матрицу (дружественный оператор)
  S21Matrix result1 = 2.0 * matrix;
  S21Matrix result2 = matrix * 2.0;
  
  EXPECT_TRUE(result1 == result2);
  EXPECT_DOUBLE_EQ(result1(0, 0), 2.0);
  EXPECT_DOUBLE_EQ(result1(0, 1), 4.0);
  EXPECT_DOUBLE_EQ(result1(1, 0), 6.0);
  EXPECT_DOUBLE_EQ(result1(1, 1), 8.0);
}

// ==================== ТЕСТЫ ПАМЯТИ И ИСКЛЮЧЕНИЙ ====================

TEST(MemoryTest, LargeMatrix) {
  S21Matrix matrix(100, 100);
  for (int i = 0; i < 100; ++i) {
    for (int j = 0; j < 100; ++j) {
      matrix(i, j) = i + j;
    }
  }

  S21Matrix copy = matrix;
  EXPECT_TRUE(matrix == copy);

  // Умножение на скаляр
  matrix *= 2.0;
  EXPECT_DOUBLE_EQ(matrix(50, 50), 200.0);
}

TEST(MemoryTest, MultipleResize) {
  S21Matrix matrix(10, 10);

  // Многократное изменение размеров
  for (int i = 0; i < 10; ++i) {
    matrix.set_rows(i + 10);
    matrix.set_cols(i + 20);
  }

  EXPECT_EQ(matrix.get_rows(), 19);
  EXPECT_EQ(matrix.get_cols(), 29);
}

TEST(ExceptionTest, InvalidOperations) {
  S21Matrix matrix1(2, 2);
  S21Matrix matrix2(3, 3);

  // Сложение разных размеров
  EXPECT_THROW(matrix1.SumMatrix(matrix2), std::invalid_argument);

  // Умножение несовместимых матриц
  EXPECT_THROW(matrix1.MulMatrix(matrix2), std::invalid_argument);

  // Определитель неквадратной матрицы
  S21Matrix non_square(2, 3);
  EXPECT_THROW(non_square.Determinant(), std::logic_error);

  // Обратная матрица вырожденной матрицы
  S21Matrix singular(2, 2);
  singular(0, 0) = 1.0;
  singular(0, 1) = 2.0;
  singular(1, 0) = 2.0;
  singular(1, 1) = 4.0;
  EXPECT_THROW(singular.InverseMatrix(), std::logic_error);
}

// ==================== ДОПОЛНИТЕЛЬНЫЕ ТЕСТЫ ДЛЯ УВЕЛИЧЕНИЯ ПОКРЫТИЯ ====================

TEST(AdditionalTests, InvalidMatrixOperations) {
    // Тест для пустой матрицы после перемещения
    S21Matrix matrix1(2, 2);
    matrix1(0, 0) = 1.0;
    S21Matrix matrix2(std::move(matrix1));
    
    // Проверяем, что исходная матрица теперь невалидна
    EXPECT_EQ(matrix1.get_rows(), 0);
    EXPECT_EQ(matrix1.get_cols(), 0);
    EXPECT_THROW(matrix1(0, 0), std::out_of_range);
    
    // Тест MulNumber на невалидной матрице (не должно бросать исключение, просто выйти)
    S21Matrix invalid_matrix;
    invalid_matrix = std::move(matrix1); // matrix1 уже пустая после перемещения
    // MulNumber для невалидной матрицы просто возвращает управление
    EXPECT_NO_THROW(invalid_matrix.MulNumber(5.0));
}

TEST(AdditionalTests, EdgeCaseMatrices) {
    // Матрица 1x1 дополнения (должна выбрасывать исключение)
    S21Matrix matrix1x1(1, 1);
    matrix1x1(0, 0) = 5.0;
    EXPECT_THROW(matrix1x1.CalcComplements(), std::logic_error);
    
    // Определитель матрицы 1x1
    EXPECT_DOUBLE_EQ(matrix1x1.Determinant(), 5.0);
    
    // Обратная матрица 1x1
    S21Matrix inverse1x1 = matrix1x1.InverseMatrix();
    EXPECT_DOUBLE_EQ(inverse1x1(0, 0), 0.2);
}

TEST(AdditionalTests, GaussianEliminationEdgeCases) {
    // Матрица с нулевым столбцом (определитель должен быть 0)
    S21Matrix zero_col_matrix(3, 3);
    zero_col_matrix(0, 0) = 1.0; zero_col_matrix(0, 1) = 0.0; zero_col_matrix(0, 2) = 3.0;
    zero_col_matrix(1, 0) = 4.0; zero_col_matrix(1, 1) = 0.0; zero_col_matrix(1, 2) = 6.0;
    zero_col_matrix(2, 0) = 7.0; zero_col_matrix(2, 1) = 0.0; zero_col_matrix(2, 2) = 9.0;
    
    EXPECT_DOUBLE_EQ(zero_col_matrix.Determinant(), 0.0);
    
    // Матрица, требующая перестановки строк
    S21Matrix swap_matrix(3, 3);
    swap_matrix(0, 0) = 0.0; swap_matrix(0, 1) = 1.0; swap_matrix(0, 2) = 2.0;
    swap_matrix(1, 0) = 3.0; swap_matrix(1, 1) = 4.0; swap_matrix(1, 2) = 5.0;
    swap_matrix(2, 0) = 6.0; swap_matrix(2, 1) = 7.0; swap_matrix(2, 2) = 8.0;
    
    // Определитель должен быть 0 (строки линейно зависимы)
    EXPECT_NEAR(swap_matrix.Determinant(), 0.0, 1e-7);
}

TEST(AdditionalTests, SetRowsColsEdgeCases) {
    // Установка того же количества строк/столбцов
    S21Matrix matrix(3, 3);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            matrix(i, j) = i * 3 + j + 1;
        }
    }
    
    // Сохраняем оригинал
    S21Matrix original = matrix;
    
    // Устанавливаем те же размеры (ничего не должно измениться)
    matrix.set_rows(3);
    matrix.set_cols(3);
    EXPECT_TRUE(matrix == original);
    
    // Уменьшение размеров
    matrix.set_rows(2);
    EXPECT_EQ(matrix.get_rows(), 2);
    EXPECT_EQ(matrix.get_cols(), 3);
    
    // Проверяем, что данные сохранились
    EXPECT_DOUBLE_EQ(matrix(0, 0), 1.0);
    EXPECT_DOUBLE_EQ(matrix(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(matrix(0, 2), 3.0);
    EXPECT_DOUBLE_EQ(matrix(1, 0), 4.0);
    EXPECT_DOUBLE_EQ(matrix(1, 1), 5.0);
    EXPECT_DOUBLE_EQ(matrix(1, 2), 6.0);
}

TEST(AdditionalTests, CopyAndMoveOperations) {
    // Тест копирования пустой матрицы (после перемещения)
    S21Matrix matrix1(2, 2);
    matrix1(0, 0) = 5.0;
    S21Matrix matrix2(std::move(matrix1)); // matrix1 теперь пустая
    
    // Копирование пустой матрицы
    S21Matrix matrix3(matrix1);
    EXPECT_EQ(matrix3.get_rows(), 0);
    EXPECT_EQ(matrix3.get_cols(), 0);
    
    // Тест самоприсваивания через операторы
    S21Matrix self_assign(2, 2);
    self_assign(0, 0) = 5.0;
    self_assign = self_assign; // Самоприсваивание
    EXPECT_DOUBLE_EQ(self_assign(0, 0), 5.0);
    
    // Самоприсваивание move
    self_assign = std::move(self_assign);
    EXPECT_DOUBLE_EQ(self_assign(0, 0), 5.0);
}

TEST(AdditionalTests, AlgebraicComplementsThorough) {
    // Матрица 2x2 для алгебраических дополнений
    S21Matrix matrix2x2(2, 2);
    matrix2x2(0, 0) = 1.0;
    matrix2x2(0, 1) = 2.0;
    matrix2x2(1, 0) = 3.0;
    matrix2x2(1, 1) = 4.0;
    
    S21Matrix complements2x2 = matrix2x2.CalcComplements();
    EXPECT_DOUBLE_EQ(complements2x2(0, 0), 4.0);
    EXPECT_DOUBLE_EQ(complements2x2(0, 1), -3.0);
    EXPECT_DOUBLE_EQ(complements2x2(1, 0), -2.0);
    EXPECT_DOUBLE_EQ(complements2x2(1, 1), 1.0);
    
    // Проверка для матрицы 4x4 (будет использован рекурсивный метод)
    S21Matrix matrix4x4(4, 4);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            matrix4x4(i, j) = i * 4 + j + 1;
        }
    }
    
    // Для этой матрицы определитель должен быть 0
    EXPECT_NEAR(matrix4x4.Determinant(), 0.0, 1e-7);
}

TEST(AdditionalTests, InverseMatrixEdgeCases) {
    // Вырожденная матрица (определитель 0)
    S21Matrix singular(2, 2);
    singular(0, 0) = 1.0;
    singular(0, 1) = 2.0;
    singular(1, 0) = 2.0;
    singular(1, 1) = 4.0;
    
    EXPECT_THROW(singular.InverseMatrix(), std::logic_error);
    
    // Матрица с очень маленьким определителем (но не нулевым)
    S21Matrix near_singular(2, 2);
    near_singular(0, 0) = 1.0;
    near_singular(0, 1) = 2.0;
    near_singular(1, 0) = 2.0;
    near_singular(1, 1) = 4.0000001;
    
    // Обратная матрица должна существовать
    S21Matrix inverse_near = near_singular.InverseMatrix();
    
    // Проверяем A * A^(-1) = I
    S21Matrix identity_check = near_singular * inverse_near;
    EXPECT_NEAR(identity_check(0, 0), 1.0, 1e-6);
    EXPECT_NEAR(identity_check(0, 1), 0.0, 1e-6);
    EXPECT_NEAR(identity_check(1, 0), 0.0, 1e-6);
    EXPECT_NEAR(identity_check(1, 1), 1.0, 1e-6);
}

TEST(AdditionalTests, LargeMatrixOperations) {
    // Тест для большой матрицы 10x10
    S21Matrix large(10, 10);
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            large(i, j) = (i == j) ? 1.0 : 0.0; // Единичная матрица
        }
    }
    
    // Определитель единичной матрицы = 1
    EXPECT_DOUBLE_EQ(large.Determinant(), 1.0);
    
    // Транспонирование единичной матрицы = сама себе
    S21Matrix transposed_large = large.Transpose();
    EXPECT_TRUE(large == transposed_large);
    
    // Обратная матрица единичной матрицы = сама себе
    S21Matrix inverse_large = large.InverseMatrix();
    EXPECT_TRUE(large == inverse_large);
}

TEST(AdditionalTests, OperatorChaining) {
    // Тест цепочки операций
    S21Matrix A(2, 2);
    A(0, 0) = 1.0; A(0, 1) = 2.0;
    A(1, 0) = 3.0; A(1, 1) = 4.0;
    
    S21Matrix B(2, 2);
    B(0, 0) = 5.0; B(0, 1) = 6.0;
    B(1, 0) = 7.0; B(1, 1) = 8.0;
    
    S21Matrix C(2, 2);
    C(0, 0) = 9.0; C(0, 1) = 10.0;
    C(1, 0) = 11.0; C(1, 1) = 12.0;
    
    // Цепочка операций: (A + B) * C
    S21Matrix result1 = (A + B) * C;
    
    // Альтернативный расчет для проверки
    S21Matrix temp = A + B;
    S21Matrix result2 = temp * C;
    
    EXPECT_TRUE(result1 == result2);
    
    // Цепочка с присваиванием: A += B *= C
    S21Matrix A_copy = A;
    S21Matrix B_copy = B;
    
    A_copy += B_copy *= C;
    
    // Проверяем вручную
    B_copy = B;
    B_copy *= C;
    S21Matrix expected = A;
    expected += B_copy;
    
    EXPECT_TRUE(A_copy == expected);
}

TEST(AdditionalTests, ExceptionMessages) {
    // Проверяем текст исключений
    try {
        S21Matrix invalid(-1, 5);
        FAIL() << "Expected std::invalid_argument";
    } catch (const std::invalid_argument& e) {
        EXPECT_STREQ(e.what(), "Rows and cols must be positive");
    }
    
    S21Matrix matrix(2, 2);
    try {
        matrix(5, 5);
        FAIL() << "Expected std::out_of_range";
    } catch (const std::out_of_range& e) {
        EXPECT_NE(std::string(e.what()).find("index is out of range"), std::string::npos);
    }
    
    S21Matrix non_square(2, 3);
    try {
        non_square.Determinant();
        FAIL() << "Expected std::logic_error";
    } catch (const std::logic_error& e) {
        EXPECT_NE(std::string(e.what()).find("square"), std::string::npos);
    }
}

TEST(AdditionalTests, ZeroAndNegativeScalar) {
    S21Matrix matrix(2, 2);
    matrix(0, 0) = 1.0; matrix(0, 1) = -2.0;
    matrix(1, 0) = -3.0; matrix(1, 1) = 4.0;
    
    // Умножение на 0
    S21Matrix zero_scaled = matrix * 0.0;
    EXPECT_DOUBLE_EQ(zero_scaled(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(zero_scaled(0, 1), 0.0);
    EXPECT_DOUBLE_EQ(zero_scaled(1, 0), 0.0);
    EXPECT_DOUBLE_EQ(zero_scaled(1, 1), 0.0);
    
    // Умножение на отрицательное число
    S21Matrix neg_scaled = matrix * -2.0;
    EXPECT_DOUBLE_EQ(neg_scaled(0, 0), -2.0);
    EXPECT_DOUBLE_EQ(neg_scaled(0, 1), 4.0);
    EXPECT_DOUBLE_EQ(neg_scaled(1, 0), 6.0);
    EXPECT_DOUBLE_EQ(neg_scaled(1, 1), -8.0);
    
    // Комбинированное: -1 * matrix
    S21Matrix neg_matrix = -1.0 * matrix;
    EXPECT_DOUBLE_EQ(neg_matrix(0, 0), -1.0);
    EXPECT_DOUBLE_EQ(neg_matrix(0, 1), 2.0);
    EXPECT_DOUBLE_EQ(neg_matrix(1, 0), 3.0);
    EXPECT_DOUBLE_EQ(neg_matrix(1, 1), -4.0);
}

TEST(AdditionalTests, MatrixProperties) {
    // Проверка свойства (A^T)^T = A
    S21Matrix A(3, 2);
    A(0, 0) = 1.0; A(0, 1) = 2.0;
    A(1, 0) = 3.0; A(1, 1) = 4.0;
    A(2, 0) = 5.0; A(2, 1) = 6.0;
    
    S21Matrix A_T = A.Transpose();
    S21Matrix A_T_T = A_T.Transpose();
    
    EXPECT_TRUE(A == A_T_T);
    
    // Проверка свойства (AB)^T = B^T A^T
    S21Matrix B(2, 3);
    B(0, 0) = 7.0; B(0, 1) = 8.0; B(0, 2) = 9.0;
    B(1, 0) = 10.0; B(1, 1) = 11.0; B(1, 2) = 12.0;
    
    S21Matrix AB = A * B;
    S21Matrix AB_T = AB.Transpose();
    
    S21Matrix B_T = B.Transpose();
    S21Matrix A_T2 = A.Transpose();
    S21Matrix B_T_A_T = B_T * A_T2;
    
    EXPECT_TRUE(AB_T.EqMatrix(B_T_A_T));
}

// ===================== Apply =====================
TEST(S21MatrixTest, ApplySigmoid) {
  s21::S21Matrix m(2, 2);
  m(0,0)=0.0; m(0,1)=1.0; m(1,0)=-1.0; m(1,1)=2.0;
  m.Apply([](double x) { return 1.0 / (1.0 + exp(-x)); });
  EXPECT_NEAR(m(0,0), 0.5, 1e-6);
  EXPECT_NEAR(m(0,1), 0.73105857863, 1e-6);
  EXPECT_NEAR(m(1,0), 0.26894142137, 1e-6);
  EXPECT_NEAR(m(1,1), 0.88079707797, 1e-6);
}

TEST(S21MatrixTest, ApplySquare) {
  s21::S21Matrix m(1, 3);
  m(0,0)=1.0; m(0,1)=2.0; m(0,2)=3.0;
  m.Apply([](double x) { return x * x; });
  EXPECT_DOUBLE_EQ(m(0,0), 1.0);
  EXPECT_DOUBLE_EQ(m(0,1), 4.0);
  EXPECT_DOUBLE_EQ(m(0,2), 9.0);
}

// ===================== Transpose =====================
TEST(S21MatrixTest, TransposeSquare) {
  s21::S21Matrix m(2, 2);
  m(0,0)=1; m(0,1)=2; m(1,0)=3; m(1,1)=4;
  s21::S21Matrix t = m.Transpose();
  EXPECT_EQ(t.GetRows(), 2);
  EXPECT_EQ(t.GetCols(), 2);
  EXPECT_DOUBLE_EQ(t(0,0), 1);
  EXPECT_DOUBLE_EQ(t(0,1), 3);
  EXPECT_DOUBLE_EQ(t(1,0), 2);
  EXPECT_DOUBLE_EQ(t(1,1), 4);
}

TEST(S21MatrixTest, TransposeRectangular) {
  s21::S21Matrix m(2, 3);
  m(0,0)=1; m(0,1)=2; m(0,2)=3;
  m(1,0)=4; m(1,1)=5; m(1,2)=6;
  s21::S21Matrix t = m.Transpose();
  EXPECT_EQ(t.GetRows(), 3);
  EXPECT_EQ(t.GetCols(), 2);
  EXPECT_DOUBLE_EQ(t(0,0), 1);
  EXPECT_DOUBLE_EQ(t(0,1), 4);
  EXPECT_DOUBLE_EQ(t(1,0), 2);
  EXPECT_DOUBLE_EQ(t(1,1), 5);
  EXPECT_DOUBLE_EQ(t(2,0), 3);
  EXPECT_DOUBLE_EQ(t(2,1), 6);
}

// ===================== HadamardProduct =====================
TEST(S21MatrixTest, HadamardProductValid) {
  s21::S21Matrix a(2, 2), b(2, 2);
  a(0,0)=1; a(0,1)=2; a(1,0)=3; a(1,1)=4;
  b(0,0)=5; b(0,1)=6; b(1,0)=7; b(1,1)=8;
  s21::S21Matrix c = a.HadamardProduct(b);
  EXPECT_DOUBLE_EQ(c(0,0), 5);
  EXPECT_DOUBLE_EQ(c(0,1), 12);
  EXPECT_DOUBLE_EQ(c(1,0), 21);
  EXPECT_DOUBLE_EQ(c(1,1), 32);
}

TEST(S21MatrixTest, HadamardProductMismatch) {
  s21::S21Matrix a(2, 3), b(3, 2);
  EXPECT_THROW(a.HadamardProduct(b), std::invalid_argument);
}

// ===================== Move Assignment =====================
TEST(S21MatrixTest, MoveAssignment) {
  s21::S21Matrix src(3, 3);
  src(0,0)=99.0;
  s21::S21Matrix dst;
  dst = std::move(src);
  EXPECT_EQ(dst.GetRows(), 3);
  EXPECT_EQ(dst.GetCols(), 3);
  EXPECT_DOUBLE_EQ(dst(0,0), 99.0);
  // Исходный объект должен быть "пустым"
  EXPECT_EQ(src.GetRows(), 0);
  EXPECT_EQ(src.GetCols(), 0);
}

// ===================== Scalar Multiplication Operators =====================
TEST(S21MatrixTest, OperatorMulScalar) {
  s21::S21Matrix m(2, 2);
  m(0,0)=1; m(0,1)=2; m(1,0)=3; m(1,1)=4;
  s21::S21Matrix r = m * 2.5;
  EXPECT_DOUBLE_EQ(r(0,0), 2.5);
  EXPECT_DOUBLE_EQ(r(0,1), 5.0);
  EXPECT_DOUBLE_EQ(r(1,0), 7.5);
  EXPECT_DOUBLE_EQ(r(1,1), 10.0);
}

TEST(S21MatrixTest, OperatorMulEqualScalar) {
  s21::S21Matrix m(2, 2);
  m(0,0)=1; m(0,1)=2; m(1,0)=3; m(1,1)=4;
  m *= 0.5;
  EXPECT_DOUBLE_EQ(m(0,0), 0.5);
  EXPECT_DOUBLE_EQ(m(0,1), 1.0);
  EXPECT_DOUBLE_EQ(m(1,0), 1.5);
  EXPECT_DOUBLE_EQ(m(1,1), 2.0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

