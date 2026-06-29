#ifndef S21_MATRIX_OOP_H
#define S21_MATRIX_OOP_H

#include <cmath>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace s21 {

class S21Matrix {
 public:
  // Конструкторы и деструктор
  S21Matrix();
  S21Matrix(int rows, int cols);
  S21Matrix(const S21Matrix& other);
  S21Matrix(S21Matrix&& other) noexcept;
  ~S21Matrix();

  // Операторы присваивания
  S21Matrix& operator=(const S21Matrix& other);
  S21Matrix& operator=(S21Matrix&& other) noexcept;

  // Доступ к элементам
  double& operator()(int row, int col);
  const double& operator()(int row, int col) const;

  // Арифметические операторы
  S21Matrix operator+(const S21Matrix& other) const;
  S21Matrix operator-(const S21Matrix& other) const;
  S21Matrix operator*(const S21Matrix& other) const;        // матричное умножение
  S21Matrix operator*(double num) const;                    // умножение на скаляр
  S21Matrix& operator+=(const S21Matrix& other);
  S21Matrix& operator-=(const S21Matrix& other);
  S21Matrix& operator*=(const S21Matrix& other);            // поэлементное (Адамар)
  S21Matrix& operator*=(double num);

  // Поэлементное применение функции (сигмоида и т.п.)
  void Apply(const std::function<double(double)>& func);

  // Транспонирование
  S21Matrix Transpose() const;

  // Произведение Адамара (поэлементное умножение)
  S21Matrix HadamardProduct(const S21Matrix& other) const;

  // Методы для работы с размерностями
  int GetRows() const;
  int GetCols() const;

  // Утилиты
  bool EqMatrix(const S21Matrix& other) const;
  void SumMatrix(const S21Matrix& other);
  void SubMatrix(const S21Matrix& other);
  void MulNumber(const double num);
  void MulMatrix(const S21Matrix& other);

 private:
  int rows_, cols_;
  double** matrix_;

  void AllocateMemory();
  void FreeMemory();
  void CopyFrom(const S21Matrix& other);
};

}  // namespace s21

#endif  // S21_MATRIX_OOP_H
