#include "trainer/simple_trainer.h"
#include "perceptron/matrix_perceptron.h"

#include <random>
#include <algorithm>
#include <cassert>

namespace s21 {
namespace mlp {

void SimpleTrainer::Train(IPerceptron& perceptron, const Dataset& train_data,
                          const Dataset& valid_data,
                          const std::function<void(int, double, double)>& on_epoch) {
  Dataset shuffled = train_data;
  std::random_device rd;
  std::mt19937 g(rd());

  for (int epoch = 0; epoch < epochs_; ++epoch) {
    if (shuffle_) {
      std::shuffle(shuffled.begin(), shuffled.end(), g);
    }

    // Обучение на одном проходе
    for (const auto& sample : shuffled) {
      static_cast<MatrixPerceptron&>(perceptron).Forward(sample.first);
      static_cast<MatrixPerceptron&>(perceptron).Backward(sample.second);
      static_cast<MatrixPerceptron&>(perceptron).UpdateWeights(learning_rate_);
    }

    // Подсчёт ошибки на обучающей и валидационной выборках
    double train_loss = 0.0;
    for (const auto& s : shuffled) {
      auto out = perceptron.Predict(s.first);
      for (size_t k = 0; k < out.size(); ++k) {
        double err = out[k] - s.second[k];
        train_loss += err * err;
      }
    }
    train_loss /= shuffled.size();

    double valid_loss = 0.0;
    if (!valid_data.empty()) {
      for (const auto& s : valid_data) {
        auto out = perceptron.Predict(s.first);
        for (size_t k = 0; k < out.size(); ++k) {
          double err = out[k] - s.second[k];
          valid_loss += err * err;
        }
      }
      valid_loss /= valid_data.size();
    }

    if (on_epoch) {
      on_epoch(epoch, train_loss, valid_loss);
    }
  }
}

double SimpleTrainer::CrossValidate(IPerceptron&, const Dataset&, int) {
  // Пока не реализовано, вернём 0.0
  throw std::logic_error("CrossValidate not implemented yet");
}

}  // namespace mlp
}  // namespace s21
