#ifndef MLP_SIMPLE_TRAINER_H
#define MLP_SIMPLE_TRAINER_H

#include "core/ITrainer.h"

namespace s21 {
namespace mlp {

// Простой тренер с SGD, перемешиванием и колбэком по эпохам.
class SimpleTrainer : public ITrainer {
 public:
  SimpleTrainer(double learning_rate, int epochs, bool shuffle = true)
      : learning_rate_(learning_rate), epochs_(epochs), shuffle_(shuffle) {}

  void Train(IPerceptron& perceptron, const Dataset& train_data,
             const Dataset& valid_data,
             const std::function<void(int, double, double)>& on_epoch) override;

  double CrossValidate(IPerceptron& perceptron, const Dataset& data,
                       int k_folds) override;

 private:
  double learning_rate_;
  int epochs_;
  bool shuffle_;
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_SIMPLE_TRAINER_H
