#include "s21_matrix_oop.h"

// ==================== Constructors and destructors ====================

S21Matrix::S21Matrix() : rows_(0), cols_(0), matrix_(nullptr) {}

S21Matrix::S21Matrix(int rows, int cols)
    : rows_(rows), cols_(cols), matrix_(nullptr) {
  if (rows <= 0 || cols <= 0) {
    throw std::invalid_argument("Matrix dimensions must be positive");
  }
  AllocateMemory();
}

S21Matrix::S21Matrix(const S21Matrix& other)
    : rows_(other.rows_), cols_(other.cols_), matrix_(nullptr) {
  AllocateMemory();
  CopyMatrix(other);
}

S21Matrix::S21Matrix(S21Matrix&& other) noexcept
    : rows_(other.rows_), cols_(other.cols_), matrix_(other.matrix_) {
  other.rows_ = 0;
  other.cols_ = 0;
  other.matrix_ = nullptr;
}

S21Matrix::~S21Matrix() { DeallocateMemory(); }

// ==================== Assignment operators ====================

S21Matrix& S21Matrix::operator=(const S21Matrix& other) {
  if (this != &other) {
    DeallocateMemory();
    rows_ = other.rows_;
    cols_ = other.cols_;
    AllocateMemory();
    CopyMatrix(other);
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

// ==================== Memory management ====================

void S21Matrix::AllocateMemory() {
  if (rows_ > 0 && cols_ > 0) {
    matrix_ = new double*[rows_];
    for (int i = 0; i < rows_; ++i) {
      matrix_[i] = new double[cols_]();
    }
  }
}

void S21Matrix::DeallocateMemory() noexcept {
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

void S21Matrix::CopyMatrix(const S21Matrix& other) noexcept {
  if (matrix_ != nullptr && other.matrix_ != nullptr) {
    for (int i = 0; i < rows_; ++i) {
      std::copy(other.matrix_[i], other.matrix_[i] + cols_, matrix_[i]);
    }
  }
}

// ==================== Accessors and mutators ====================

int S21Matrix::GetRows() const noexcept { return rows_; }
int S21Matrix::GetCols() const noexcept { return cols_; }

void S21Matrix::SetRows(int rows) {
  if (rows <= 0) {
    throw std::invalid_argument("Rows must be positive");
  }
  Resize(rows, cols_);
}

void S21Matrix::SetCols(int cols) {
  if (cols <= 0) {
    throw std::invalid_argument("Cols must be positive");
  }
  Resize(rows_, cols);
}

void S21Matrix::Resize(int rows, int cols) {
  if (rows <= 0 || cols <= 0) {
    throw std::invalid_argument("Dimensions must be positive");
  }

  if (rows != rows_ || cols != cols_) {
    S21Matrix temp(rows, cols);

    int copy_rows = std::min(rows, rows_);
    int copy_cols = std::min(cols, cols_);

    for (int i = 0; i < copy_rows; ++i) {
      for (int j = 0; j < copy_cols; ++j) {
        temp.matrix_[i][j] = matrix_[i][j];
      }
    }

    *this = std::move(temp);
  }
}

// ==================== Validation ====================

void S21Matrix::CheckMatrix() const {
  if (rows_ <= 0 || cols_ <= 0 || matrix_ == nullptr) {
    throw std::logic_error("Matrix is not initialized");
  }
}

void S21Matrix::CheckIndex(int row, int col) const {
  if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
    throw std::out_of_range("Matrix index out of range");
  }
}

void S21Matrix::CheckSquare() const {
  CheckMatrix();
  if (rows_ != cols_) {
    throw std::logic_error("Matrix must be square");
  }
}

void S21Matrix::CheckSameSize(const S21Matrix& other) const {
  CheckMatrix();
  other.CheckMatrix();
  if (rows_ != other.rows_ || cols_ != other.cols_) {
    throw std::logic_error("Matrices must have same dimensions");
  }
}

void S21Matrix::CheckMulDimensions(const S21Matrix& other) const {
  CheckMatrix();
  other.CheckMatrix();
  if (cols_ != other.rows_) {
    throw std::logic_error(
        "Columns of first matrix must equal rows of second matrix");
  }
}

// ==================== Basic operations ====================

bool S21Matrix::EqMatrix(const S21Matrix& other) const {
  if (!IsValid() || !other.IsValid() || rows_ != other.rows_ ||
      cols_ != other.cols_) {
    return false;
  }

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      double a = matrix_[i][j];
      double b = other.matrix_[i][j];
      double absA = std::fabs(a);
      double absB = std::fabs(b);
      double diff = std::fabs(a - b);
      double eps = 1e-7;
      // Относительная погрешность, но если оба числа близки к нулю, используем
      // абсолютную
      if (diff > eps && diff > eps * std::max(1.0, std::max(absA, absB))) {
        return false;
      }
    }
  }
  return true;
}

void S21Matrix::SumMatrix(const S21Matrix& other) {
  CheckSameSize(other);

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      matrix_[i][j] += other.matrix_[i][j];
    }
  }
}

void S21Matrix::SubMatrix(const S21Matrix& other) {
  CheckSameSize(other);

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      matrix_[i][j] -= other.matrix_[i][j];
    }
  }
}

void S21Matrix::MulNumber(double num) noexcept {
  if (IsValid()) {
    for (int i = 0; i < rows_; ++i) {
      for (int j = 0; j < cols_; ++j) {
        matrix_[i][j] *= num;
      }
    }
  }
}

void S21Matrix::MulMatrix(const S21Matrix& other) {
  CheckMulDimensions(other);
  S21Matrix result(rows_, other.cols_);

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < other.cols_; ++j) {
      double sum = 0.0;
      for (int k = 0; k < cols_; ++k) {
        sum += matrix_[i][k] * other.matrix_[k][j];
      }
      result.matrix_[i][j] = sum;
    }
  }

  *this = std::move(result);
}

// ==================== Advanced operations ====================

S21Matrix S21Matrix::Transpose() const {
  CheckMatrix();

  S21Matrix result(cols_, rows_);

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      result.matrix_[j][i] = matrix_[i][j];
    }
  }

  return result;
}

S21Matrix S21Matrix::CalcComplements() const {
  CheckSquare();

  if (rows_ == 1) {
    throw std::logic_error("Matrix must be at least 2x2 for complements");
  }

  S21Matrix result(rows_, cols_);

  for (int i = 0; i < rows_; ++i) {
    for (int j = 0; j < cols_; ++j) {
      S21Matrix minor = GetMinor(i, j);
      double det = minor.Determinant();
      result.matrix_[i][j] = ((i + j) % 2 == 0 ? 1 : -1) * det;
    }
  }

  return result;
}

double S21Matrix::Determinant() const {
  CheckSquare();

  if (rows_ == 1) {
    return matrix_[0][0];
  }

  if (rows_ == 2) {
    return matrix_[0][0] * matrix_[1][1] - matrix_[0][1] * matrix_[1][0];
  }

  double det = 0.0;

  for (int j = 0; j < cols_; ++j) {
    S21Matrix minor = GetMinor(0, j);
    double minor_det = minor.Determinant();
    det += ((j % 2 == 0) ? 1 : -1) * matrix_[0][j] * minor_det;
  }

  return det;
}

S21Matrix S21Matrix::InverseMatrix() const {
  CheckSquare();

  if (rows_ == 1) {
    if (fabs(matrix_[0][0]) < 1e-7) {
      throw std::logic_error("Matrix is singular (determinant is zero)");
    }
    S21Matrix result(1, 1);
    result.matrix_[0][0] = 1.0 / matrix_[0][0];
    return result;
  }

  double det = Determinant();
  if (std::fabs(det) < 1e-7) {
    throw std::logic_error("Matrix is singular (determinant is zero)");
  }

  S21Matrix complements = CalcComplements();
  S21Matrix transposed = complements.Transpose();

  transposed.MulNumber(1.0 / det);
  return transposed;
}

// ==================== Helper methods ====================

S21Matrix S21Matrix::GetMinor(int excluded_row, int excluded_col) const {
  S21Matrix minor(rows_ - 1, cols_ - 1);

  int minor_i = 0;
  for (int i = 0; i < rows_; ++i) {
    if (i == excluded_row) continue;

    int minor_j = 0;
    for (int j = 0; j < cols_; ++j) {
      if (j == excluded_col) continue;

      minor.matrix_[minor_i][minor_j] = matrix_[i][j];
      ++minor_j;
    }
    ++minor_i;
  }

  return minor;
}

bool S21Matrix::IsValid() const noexcept {
  return rows_ > 0 && cols_ > 0 && matrix_ != nullptr;
}

bool S21Matrix::IsSquare() const noexcept {
  return IsValid() && rows_ == cols_;
}

void S21Matrix::Print() const noexcept {
  if (IsValid()) {
    for (int i = 0; i < rows_; ++i) {
      for (int j = 0; j < cols_; ++j) {
        std::cout << matrix_[i][j] << " ";
      }
      std::cout << std::endl;
    }
  }
}

// ==================== Operator overloads ====================

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

S21Matrix S21Matrix::operator*(double num) const {
  S21Matrix result(*this);
  result.MulNumber(num);
  return result;
}

S21Matrix operator*(double num, const S21Matrix& matrix) {
  return matrix * num;
}

bool S21Matrix::operator==(const S21Matrix& other) const noexcept {
  return EqMatrix(other);
}

bool S21Matrix::operator!=(const S21Matrix& other) const noexcept {
  return !(*this == other);
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

S21Matrix& S21Matrix::operator*=(double num) noexcept {
  MulNumber(num);
  return *this;
}

double& S21Matrix::operator()(int row, int col) {
  CheckIndex(row, col);
  return matrix_[row][col];
}

const double& S21Matrix::operator()(int row, int col) const {
  CheckIndex(row, col);
  return matrix_[row][col];
}
