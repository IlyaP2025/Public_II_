#ifndef MLP_ITRAINER_H
#define MLP_ITRAINER_H

#include <vector>
#include <functional>

#include "IPerceptron.h"

namespace s21 {
namespace mlp {

// Один обучающий пример: (входной вектор, целевой выходной вектор).
using DataSample = std::pair<std::vector<double>, std::vector<double>>;

// Набор данных (обучающая или тестовая выборка).
using Dataset = std::vector<DataSample>;

// Интерфейс алгоритма обучения.
class ITrainer {
 public:
  virtual ~ITrainer() = default;

  // Запустить обучение перцептрона.
  // on_epoch вызывается после каждой эпохи: номер эпохи, ошибка на train, ошибка на valid.
  virtual void Train(IPerceptron& perceptron,
                     const Dataset& train_data,
                     const Dataset& valid_data,
                     const std::function<void(int epoch, double train_loss, double valid_loss)>& on_epoch) = 0;

  // Кросс-валидация по k фолдам. Возвращает среднюю точность.
  virtual double CrossValidate(IPerceptron& perceptron,
                               const Dataset& data,
                               int k_folds) = 0;
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_ITRAINER_H
