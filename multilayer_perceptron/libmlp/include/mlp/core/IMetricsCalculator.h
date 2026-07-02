#ifndef MLP_IMETRICSCALCULATOR_H
#define MLP_IMETRICSCALCULATOR_H

#include <vector>

namespace s21 {
namespace mlp {

// Интерфейс для расчёта метрик качества классификации.
class IMetricsCalculator {
 public:
  virtual ~IMetricsCalculator() = default;

  // Общая точность (доля правильных ответов).
  virtual double Accuracy(const std::vector<int>& predicted, const std::vector<int>& actual) = 0;

  // Точность для заданного класса (Precision).
  virtual double Precision(const std::vector<int>& predicted, const std::vector<int>& actual, int class_label) = 0;

  // Полнота для заданного класса (Recall).
  virtual double Recall(const std::vector<int>& predicted, const std::vector<int>& actual, int class_label) = 0;

  // F-мера для заданного класса (beta по умолчанию 1.0).
  virtual double FMeasure(const std::vector<int>& predicted, const std::vector<int>& actual, int class_label, double beta = 1.0) = 0;
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_IMETRICSCALCULATOR_H
