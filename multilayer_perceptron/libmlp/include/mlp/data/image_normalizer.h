#ifndef MLP_IMAGE_NORMALIZER_H
#define MLP_IMAGE_NORMALIZER_H

#include <vector>

namespace s21 {
namespace mlp {

class ImageNormalizer {
 public:
  // Принимает вектор 784 пикселей (28x28, значения 0..1, фон чёрный).
  // Возвращает нормализованный вектор того же размера.
  static std::vector<double> Normalize(const std::vector<double>& input);
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_IMAGE_NORMALIZER_H
