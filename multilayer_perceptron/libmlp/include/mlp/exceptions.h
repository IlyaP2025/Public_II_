#ifndef MLP_EXCEPTIONS_H
#define MLP_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace s21 {
namespace mlp {

// Базовое исключение всей библиотеки MLP.
class MlpException : public std::runtime_error {
 public:
  explicit MlpException(const std::string& msg) : std::runtime_error(msg) {}
};

// Несоответствие размеров матриц / векторов.
class DimensionMismatch : public MlpException {
 public:
  explicit DimensionMismatch(const std::string& msg) : MlpException(msg) {}
};

// Ошибка загрузки файла (CSV, BMP, весов и т.п.).
class FileLoadError : public MlpException {
 public:
  explicit FileLoadError(const std::string& msg) : MlpException(msg) {}
};

// Некорректное состояние перцептрона (например, Predict до обучения).
class InvalidPerceptronState : public MlpException {
 public:
  explicit InvalidPerceptronState(const std::string& msg) : MlpException(msg) {}
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_EXCEPTIONS_H
