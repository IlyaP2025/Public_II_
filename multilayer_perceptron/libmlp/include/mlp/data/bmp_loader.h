#ifndef MLP_BMP_LOADER_H
#define MLP_BMP_LOADER_H

#include <vector>
#include <string>

namespace s21 {
namespace mlp {

class BmpLoader {
 public:
  // Загружает BMP-файл, приводит к 28x28 серого, нормализует в [0,1],
  // применяет EMNIST-преобразование (поворот + отражение).
  // Возвращает вектор из 784 элементов.
  static std::vector<double> LoadImage(const std::string& path);
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_BMP_LOADER_H
