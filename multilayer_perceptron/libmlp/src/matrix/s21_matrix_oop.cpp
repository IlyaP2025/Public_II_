#include "matrix/s21_matrix_oop.h"

// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================

bool S21Matrix::CheckValidity() const {
  bool is_valid = false;
  if (matrix_ != nullptr && rows_ > 0 && cols_ > 0) {
    is_valid = true;
  }
  return is_valid;
}

bool S21Matrix::CheckSquare() const {
  bool is_square = false;
  if (CheckValidity() && rows_ == cols_) {
    is_square = true;
  }
  return is_square;
}

void S21Matrix::AllocateMemory() {
  if (rows_ <= 0 || cols_ <= 0) {
    throw std::invalid_argument("Matrix dimensions must be positive");
  }

  matrix_ = new double*[rows_];
  for (int i = 0; i < rows_; ++i) {
    matrix_[i] = new double[cols_]();
  }
}

void S21Matrix::DeallocateMemory() {
  if (matrix_ != nullptr) {
    for (int i = 0; i < rows_; ++i) {
      delete[] matrix_[i];
    }
    delete[] matrix_;
    matrix_ = nullptr;
  }
  rows_ = 0;
  cols_ = 0;
}

void S21Matrix::SafeCopy(const S21Matrix& other) {
  // Сначала создаем временную матрицу
  double** new_matrix = new double*[other.rows_];
  int i = 0;
  bool allocation_failed = false;
  
  // Аллоцируем строки
  while (i < other.rows_ && !allocation_failed) {
    try {
      new_matrix[i] = new double[other.cols_];
      ++i;
    } catch (...) {
      allocation_failed = true;
    }
  }
  
  // Если аллокация не удалась, очищаем и бросаем исключение
  if (allocation_failed) {
    for (int j = 0; j < i; ++j) {
      delete[] new_matrix[j];
    }
    delete[] new_matrix;
    throw std::bad_alloc();
  }
  
  // Копируем данные
  for (i = 0; i < other.rows_; ++i) {
    for (int j = 0; j < other.cols_; ++j) {
      new_matrix[i][j] = other.matrix_[i][j];
    }
  }
  
  // Освобождаем старую память
  DeallocateMemory();
  
  // Присваиваем новые значения
  rows_ = other.rows_;
  cols_ = other.cols_;
  matrix_ = new_matrix;
}

S21Matrix S21Matrix::GetMinorMatrix(int exclude_row, int exclude_col) const {
  S21Matrix minor(rows_ - 1, cols_ - 1);
  int minor_row = 0;

  for (int i = 0; i < rows_; ++i) {
    if (i == exclude_row) {
      continue;
    }
    
    int minor_col = 0;
    for (int j = 0; j < cols_; ++j) {
      if (j == exclude_col) {
        continue;
      }
      minor.matrix_[minor_row][minor_col] = matrix_[i][j];
      ++minor_col;
    }
    ++minor_row;
  }

  return minor;
}

void S21Matrix::MultiplyInternal(const S21Matrix& other,
                                 S21Matrix* result) const {
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < other.cols_; ++j) {
      double sum = 0.0;
      for (int k = 0; k < cols_; ++k) {
        sum += matrix_[i][k] * other.matrix_[k][j];
      }
      result->matrix_[i][j] = sum;
    }
  }
}

void S21Matrix::CalculateAlgebraicComplements(S21Matrix* result) const {
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      S21Matrix minor = GetMinorMatrix(i, j);
      double det = minor.Determinant();
      double sign = ((i + j) % 2 == 0) ? 1.0 : -1.0;
      result->matrix_[i][j] = sign * det;
    }
  }
}

// ==================== МЕТОДЫ ДЛЯ ОПРЕДЕЛИТЕЛЯ ====================

double S21Matrix::CalculateDeterminant1x1() const {
  double result = 0.0;
  if (CheckValidity()) {
    result = matrix_[0][0];
  }
  return result;
}

double S21Matrix::CalculateDeterminant2x2() const {
  double result = 0.0;
  if (CheckValidity() && rows_ == 2 && cols_ == 2) {
    result = matrix_[0][0] * matrix_[1][1] - matrix_[0][1] * matrix_[1][0];
  }
  return result;
}

double S21Matrix::CalculateDeterminantRecursive() const {
  double det = 0.0;
  
  if (rows_ == 1) {
    det = CalculateDeterminant1x1();
  } else if (rows_ == 2) {
    det = CalculateDeterminant2x2();
  } else {
    int sign = 1;
    for (int j = 0; j < cols_; ++j) {
      S21Matrix minor = GetMinorMatrix(0, j);
      det += sign * matrix_[0][j] * minor.Determinant();
      sign = -sign;
    }
  }
  
  return det;
}

double S21Matrix::CalculateDeterminantGaussian() const {
  S21Matrix matrix_copy(*this);
  int sign_multiplier = 1;
  bool is_zero_det = false;
  
  PerformGaussianElimination(&matrix_copy, &sign_multiplier, &is_zero_det);
  
  double result = 0.0;
  if (is_zero_det) {
    result = 0.0;
  } else {
    double product = 1.0;
    for (int i = 0; i < rows_; ++i) {
      product *= matrix_copy.matrix_[i][i];
    }
    result = sign_multiplier * product;
  }
  
  return result;
}

void S21Matrix::PerformGaussianElimination(S21Matrix* matrix_copy,
                                           int* sign_multiplier,
                                           bool* is_zero_det) const {
  const double EPSILON = 1e-7;
  
  for (int i = 0; i < rows_ - 1; ++i) {
    // Поиск максимального элемента в столбце
    double max_val = std::abs((*matrix_copy)(i, i));
    int max_row = i;
    
    for (int k = i + 1; k < rows_; ++k) {
      double abs_val = std::abs((*matrix_copy)(k, i));
      if (abs_val > max_val) {
        max_val = abs_val;
        max_row = k;
      }
    }
    
    // Если максимальный элемент почти 0
    if (max_val < EPSILON) {
      *is_zero_det = true;
      return;
    }
    
    // Перестановка строк
    if (max_row != i) {
      matrix_copy->SwapMatrixRows(i, max_row);
      *sign_multiplier *= -1;
    }
    
    // Исключение
    matrix_copy->EliminateRowsBelow(i);
  }
}

void S21Matrix::SwapMatrixRows(int row1, int row2) {
  if (row1 < 0 || row1 >= rows_ || row2 < 0 || row2 >= rows_) {
    throw std::out_of_range("Row index out of range");
  }
  
  double* temp = matrix_[row1];
  matrix_[row1] = matrix_[row2];
  matrix_[row2] = temp;
}

void S21Matrix::EliminateRowsBelow(int pivot_row) {
  const double EPSILON = 1e-7;
  
  for (int i = pivot_row + 1; i < rows_; ++i) {
    if (std::abs(matrix_[pivot_row][pivot_row]) < EPSILON) {
      continue;
    }
    
    double factor = matrix_[i][pivot_row] / matrix_[pivot_row][pivot_row];
    for (int j = pivot_row; j < cols_; ++j) {
      matrix_[i][j] -= factor * matrix_[pivot_row][j];
    }
  }
}

// ==================== КОНСТРУКТОРЫ И ДЕСТРУКТОР ====================

S21Matrix::S21Matrix() : rows_(3), cols_(3), matrix_(nullptr) {
  AllocateMemory();
}

S21Matrix::S21Matrix(int rows, int cols)
    : rows_(rows), cols_(cols), matrix_(nullptr) {
  if (rows <= 0 || cols <= 0) {
    throw std::invalid_argument("Rows and cols must be positive");
  }
  AllocateMemory();
}

S21Matrix::S21Matrix(const S21Matrix& other)
    : rows_(0), cols_(0), matrix_(nullptr) {
  SafeCopy(other);
}

S21Matrix::S21Matrix(S21Matrix&& other) noexcept
    : rows_(other.rows_), cols_(other.cols_), matrix_(other.matrix_) {
  other.rows_ = 0;
  other.cols_ = 0;
  other.matrix_ = nullptr;
}

S21Matrix::~S21Matrix() {
  DeallocateMemory();
}

// ==================== ОПЕРАТОРЫ ПРИСВАИВАНИЯ ====================

S21Matrix& S21Matrix::operator=(const S21Matrix& other) {
  if (this != &other) {
    SafeCopy(other);
  }
  return *this;
}

S21Matrix& S21Matrix::operator=(S21Matrix&& other) noexcept {
  if (this != &other) {
    DeallocateMemory();
    
    rows_ = other.rows_;
    cols_ = other.cols_;
    matrix_ = other.matrix_;
    
    other.rows_ = 0;
    other.cols_ = 0;
    other.matrix_ = nullptr;
  }
  return *this;
}

// ==================== ACCESSOR И MUTATOR ====================

int S21Matrix::get_rows() const { return rows_; }
int S21Matrix::get_cols() const { return cols_; }

void S21Matrix::set_rows(int rows) {
  if (rows <= 0) {
    throw std::invalid_argument("Rows must be positive");
  }
  
  if (rows == rows_) {
    return;
  }
  
  // СОХРАНЯЕМ старые значения
  int old_rows = rows_;
  int old_cols = cols_;
  
  // Создаем новый массив указателей
  double** new_matrix = new double*[rows];
  
  // Инициализируем новые строки
  for (int i = 0; i < rows; ++i) {
    new_matrix[i] = new double[old_cols]();
    if (i < old_rows) {
      // Копируем существующие строки
      for (int j = 0; j < old_cols; ++j) {
        new_matrix[i][j] = matrix_[i][j];
      }
    }
  }
  
  // Освобождаем старую память
  if (matrix_ != nullptr) {
    for (int i = 0; i < old_rows; ++i) {
      delete[] matrix_[i];
    }
    delete[] matrix_;
  }
  
  // Устанавливаем новые значения
  rows_ = rows;
  cols_ = old_cols; // Сохраняем количество столбцов
  matrix_ = new_matrix;
}

void S21Matrix::set_cols(int cols) {
  if (cols <= 0) {
    throw std::invalid_argument("Cols must be positive");
  }
  
  if (cols == cols_) {
    return;
  }
  
  // СОХРАНЯЕМ старые значения
  int old_rows = rows_;
  int old_cols = cols_;
  
  // Создаем новый массив указателей
  double** new_matrix = new double*[old_rows];
  
  // Инициализируем строки с новым количеством столбцов
  for (int i = 0; i < old_rows; ++i) {
    new_matrix[i] = new double[cols]();
    
    // Копируем существующие данные
    int cols_to_copy = std::min(cols, old_cols);
    for (int j = 0; j < cols_to_copy; ++j) {
      new_matrix[i][j] = matrix_[i][j];
    }
  }
  
  // Освобождаем старую память
  if (matrix_ != nullptr) {
    for (int i = 0; i < old_rows; ++i) {
      delete[] matrix_[i];
    }
    delete[] matrix_;
  }
  
  // Устанавливаем новые значения
  rows_ = old_rows; // Сохраняем количество строк
  cols_ = cols;
  matrix_ = new_matrix;
}

// ==================== ОСНОВНЫЕ ОПЕРАЦИИ С МАТРИЦАМИ ====================

bool S21Matrix::EqMatrix(const S21Matrix& other) const {
  bool is_equal = true;
  
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    is_equal = false;
  } else {
    const double EPSILON = 1e-7;
    for (int i = 0; i < rows_ && is_equal; ++i) {
      for (int j = 0; j < cols_ && is_equal; ++j) {
        if (std::fabs(matrix_[i][j] - other.matrix_[i][j]) > EPSILON) {
          is_equal = false;
        }
      }
    }
  }
  
  return is_equal;
}

void S21Matrix::SumMatrix(const S21Matrix& other) {
  if (!CheckValidity() || !other.CheckValidity()) {
    throw std::invalid_argument("Matrices must be valid");
  }
  
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw std::invalid_argument("Matrix dimensions must be equal for addition");
  }
  
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      matrix_[i][j] += other.matrix_[i][j];
    }
  }
}

void S21Matrix::SubMatrix(const S21Matrix& other) {
  if (!CheckValidity() || !other.CheckValidity()) {
    throw std::invalid_argument("Matrices must be valid");
  }
  
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw std::invalid_argument(
        "Matrix dimensions must be equal for subtraction");
  }
  
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      matrix_[i][j] -= other.matrix_[i][j];
    }
  }
}

void S21Matrix::MulNumber(double number) {
  if (!CheckValidity()) {
    return;
  }
  
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      matrix_[i][j] *= number;
    }
  }
}

void S21Matrix::MulMatrix(const S21Matrix& other) {
  if (!CheckValidity() || !other.CheckValidity()) {
    throw std::invalid_argument("Matrices must be valid");
  }
  
  if (cols_ != other.rows_) {
    throw std::invalid_argument(
        "Number of columns of the first matrix must equal "
        "number of rows of the second matrix");
  }
  
  S21Matrix result(rows_, other.cols_);
  MultiplyInternal(other, &result);
  *this = std::move(result);
}

S21Matrix S21Matrix::Transpose() const {
  if (!CheckValidity()) {
    throw std::invalid_argument("Matrix must be valid");
  }
  
  S21Matrix result(cols_, rows_);
  
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      result.matrix_[j][i] = matrix_[i][j];
    }
  }
  
  return result;
}

S21Matrix S21Matrix::CalcComplements() const {
  if (!CheckValidity()) {
    throw std::invalid_argument("Matrix must be valid");
  }
  
  if (!CheckSquare()) {
    throw std::logic_error("Matrix must be square to calculate complements");
  }
  
  if (rows_ == 1) {
    throw std::logic_error("Matrix 1x1 doesn't have complements");
  }
  
  S21Matrix result(rows_, cols_);
  CalculateAlgebraicComplements(&result);
  
  return result;
}

double S21Matrix::Determinant() const {
  if (!CheckValidity()) {
    throw std::invalid_argument("Matrix must be valid");
  }
  
  if (!CheckSquare()) {
    throw std::logic_error("Matrix must be square to calculate determinant");
  }
  
  double result = 0.0;
  if (rows_ <= 4) {
    result = CalculateDeterminantRecursive();
  } else {
    result = CalculateDeterminantGaussian();
  }
  
  return result;
}

S21Matrix S21Matrix::InverseMatrix() const {
  if (!CheckValidity()) {
    throw std::invalid_argument("Matrix must be valid");
  }
  
  if (!CheckSquare()) {
    throw std::logic_error("Matrix must be square to calculate inverse");
  }
  
  double det = Determinant();
  if (std::fabs(det) < 1e-7) {
    throw std::logic_error(
        "Matrix determinant is zero, inverse does not exist");
  }
  
  S21Matrix result;
  if (rows_ == 1) {
    result = S21Matrix(1, 1);
    result.matrix_[0][0] = 1.0 / det;
  } else {
    S21Matrix complements = CalcComplements();
    S21Matrix transposed = complements.Transpose();
    transposed.MulNumber(1.0 / det);
    result = transposed;
  }
  
  return result;
}

// ==================== ПЕРЕГРУЗКА ОПЕРАТОРОВ ====================

S21Matrix S21Matrix::operator+(const S21Matrix& other) const {
  S21Matrix result(*this);
  result.SumMatrix(other);
  return result;
}

S21Matrix S21Matrix::operator-(const S21Matrix& other) const {
  S21Matrix result(*this);
  result.SubMatrix(other);
  return result;
}

S21Matrix S21Matrix::operator*(const S21Matrix& other) const {
  S21Matrix result(*this);
  result.MulMatrix(other);
  return result;
}

S21Matrix S21Matrix::operator*(double number) const {
  S21Matrix result(*this);
  result.MulNumber(number);
  return result;
}

bool S21Matrix::operator==(const S21Matrix& other) const {
  return EqMatrix(other);
}

S21Matrix& S21Matrix::operator+=(const S21Matrix& other) {
  SumMatrix(other);
  return *this;
}

S21Matrix& S21Matrix::operator-=(const S21Matrix& other) {
  SubMatrix(other);
  return *this;
}

S21Matrix& S21Matrix::operator*=(const S21Matrix& other) {
  MulMatrix(other);
  return *this;
}

S21Matrix& S21Matrix::operator*=(double number) {
  MulNumber(number);
  return *this;
}

// ==================== ОПЕРАТОРЫ ИНДЕКСАЦИИ ====================

double& S21Matrix::operator()(int row, int col) {
  if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
    throw std::out_of_range("Incorrect input, index is out of range");
  }
  return matrix_[row][col];
}

const double& S21Matrix::operator()(int row, int col) const {
  if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
    throw std::out_of_range("Incorrect input, index is out of range");
  }
  return matrix_[row][col];
}

// ==================== ДРУЖЕСТВЕННЫЕ ОПЕРАТОРЫ ====================

S21Matrix operator*(double num, const S21Matrix& matrix) {
  return matrix * num;
}

// ==================== ДОПОЛНИТЕЛЬНЫЕ ОПЕРАЦИИ ДЛЯ MLP ====================

void S21Matrix::Apply(const std::function<double(double)>& func) {
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      matrix_[i][j] = func(matrix_[i][j]);
    }
  }
}

S21Matrix S21Matrix::HadamardProduct(const S21Matrix& other) const {
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw std::invalid_argument(
        "Matrices must have same dimensions for Hadamard product");
  }
  S21Matrix result(rows_, cols_);
  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      result.matrix_[i][j] = matrix_[i][j] * other.matrix_[i][j];
    }
  }
  return result;
}


