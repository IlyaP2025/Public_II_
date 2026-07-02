#ifndef S21_MATRIX_OOP_H
#define S21_MATRIX_OOP_H

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>

class S21Matrix {
 private:
  int rows_;
  int cols_;
  double** matrix_;

  // Вспомогательные приватные методы
  bool CheckValidity() const;
  bool CheckSquare() const;
  void AllocateMemory();
  void DeallocateMemory();
  void SafeCopy(const S21Matrix& other);
  S21Matrix GetMinorMatrix(int exclude_row, int exclude_col) const;
  void MultiplyInternal(const S21Matrix& other, S21Matrix* result) const;
  void CalculateAlgebraicComplements(S21Matrix* result) const;

  // Методы для определителя
  double CalculateDeterminant1x1() const;
  double CalculateDeterminant2x2() const;
  double CalculateDeterminantRecursive() const;
  double CalculateDeterminantGaussian() const;
  void PerformGaussianElimination(S21Matrix* matrix_copy, int* sign_multiplier,
                                  bool* is_zero_det) const;
  void SwapMatrixRows(int row1, int row2);
  void EliminateRowsBelow(int pivot_row);

 public:
  // Конструкторы и деструктор
  S21Matrix();                    // Базовый конструктор (3x3)
  S21Matrix(int rows, int cols);  // Параметризированный конструктор
  S21Matrix(const S21Matrix& other);  // Конструктор копирования
  S21Matrix(S21Matrix&& other) noexcept;  // Конструктор перемещения
  ~S21Matrix();                           // Деструктор

  // Операторы присваивания
  S21Matrix& operator=(const S21Matrix& other);
  S21Matrix& operator=(S21Matrix&& other) noexcept;

  // Accessor и Mutator
  int get_rows() const;
  int get_cols() const;
  void set_rows(int rows);
  void set_cols(int cols);

  // Основные операции с матрицами
  bool EqMatrix(const S21Matrix& other) const;
  void SumMatrix(const S21Matrix& other);
  void SubMatrix(const S21Matrix& other);
  void MulNumber(double number);
  void MulMatrix(const S21Matrix& other);
  S21Matrix Transpose() const;
  S21Matrix CalcComplements() const;
  double Determinant() const;
  S21Matrix InverseMatrix() const;

  // Перегрузка операторов
  S21Matrix operator+(const S21Matrix& other) const;
  S21Matrix operator-(const S21Matrix& other) const;
  S21Matrix operator*(const S21Matrix& other) const;
  S21Matrix operator*(double number) const;
  bool operator==(const S21Matrix& other) const;
  S21Matrix& operator+=(const S21Matrix& other);
  S21Matrix& operator-=(const S21Matrix& other);
  S21Matrix& operator*=(const S21Matrix& other);
  S21Matrix& operator*=(double number);

  // Операторы индексации
  double& operator()(int row, int col);
  const double& operator()(int row, int col) const;

  // Дружественные операторы
  friend S21Matrix operator*(double num, const S21Matrix& matrix);
};

// Дружественный оператор умножения числа на матрицу
S21Matrix operator*(double num, const S21Matrix& matrix);

#endif  // S21_MATRIX_OOP_H

