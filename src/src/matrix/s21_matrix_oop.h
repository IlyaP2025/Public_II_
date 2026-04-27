#ifndef S21_MATRIX_OOP_H
#define S21_MATRIX_OOP_H

#include <algorithm>
#include <cmath>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>

class S21Matrix {
 private:
  // Основные поля
  int rows_;
  int cols_;
  double** matrix_;

  // Приватные вспомогательные методы
  void AllocateMemory();
  void DeallocateMemory() noexcept;
  void CopyMatrix(const S21Matrix& other) noexcept;
  void CheckMatrix() const;
  void CheckIndex(int row, int col) const;
  void CheckSquare() const;
  void CheckSameSize(const S21Matrix& other) const;
  void CheckMulDimensions(const S21Matrix& other) const;
  S21Matrix GetMinor(int excluded_row, int excluded_col) const;

 public:
  // Конструкторы и деструкторы (Правило пяти)
  S21Matrix();
  explicit S21Matrix(int rows, int cols);
  S21Matrix(const S21Matrix& other);
  S21Matrix(S21Matrix&& other) noexcept;
  ~S21Matrix();

  // Операторы присваивания
  S21Matrix& operator=(const S21Matrix& other);
  S21Matrix& operator=(S21Matrix&& other) noexcept;

  // Accessors и Mutators
  int GetRows() const noexcept;
  int GetCols() const noexcept;
  void SetRows(int rows);
  void SetCols(int cols);
  void Resize(int rows, int cols);

  // Основные операции
  bool EqMatrix(const S21Matrix& other) const;
  void SumMatrix(const S21Matrix& other);
  void SubMatrix(const S21Matrix& other);
  void MulNumber(double num) noexcept;
  void MulMatrix(const S21Matrix& other);

  // Продвинутые операции
  S21Matrix Transpose() const;
  S21Matrix CalcComplements() const;
  double Determinant() const;
  S21Matrix InverseMatrix() const;

  // Перегрузка операторов
  S21Matrix operator+(const S21Matrix& other) const;
  S21Matrix operator-(const S21Matrix& other) const;
  S21Matrix operator*(const S21Matrix& other) const;
  S21Matrix operator*(double num) const;
  friend S21Matrix operator*(double num, const S21Matrix& matrix);

  bool operator==(const S21Matrix& other) const noexcept;
  bool operator!=(const S21Matrix& other) const noexcept;
  S21Matrix& operator+=(const S21Matrix& other);
  S21Matrix& operator-=(const S21Matrix& other);
  S21Matrix& operator*=(const S21Matrix& other);
  S21Matrix& operator*=(double num) noexcept;

  double& operator()(int row, int col);
  const double& operator()(int row, int col) const;

  // Вспомогательные методы
  void Print() const noexcept;
  bool IsValid() const noexcept;
  bool IsSquare() const noexcept;
};

#endif  // S21_MATRIX_OOP_H