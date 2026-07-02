#ifndef MLP_IPERCEPTRON_H
#define MLP_IPERCEPTRON_H

#include <vector>
#include <cstddef>

namespace s21 {
namespace mlp {

// Интерфейс перцептрона – контракт для матричной, графовой и будущих реализаций.
class IPerceptron {
 public:
  virtual ~IPerceptron() = default;

  // Прямой проход (предсказание) для одного входного вектора.
  virtual std::vector<double> Predict(const std::vector<double>& input) = 0;

  // Заменить все веса перцептрона.
  virtual void SetWeights(const std::vector<std::vector<double>>& weights) = 0;

  // Получить текущие веса.
  virtual std::vector<std::vector<double>> GetWeights() const = 0;

  // Размер входного вектора.
  virtual size_t InputSize() const = 0;

  // Размер выходного вектора (число классов).
  virtual size_t OutputSize() const = 0;
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_IPERCEPTRON_H
