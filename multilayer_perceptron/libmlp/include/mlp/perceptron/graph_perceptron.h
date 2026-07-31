#ifndef MLP_GRAPH_PERCEPTRON_H
#define MLP_GRAPH_PERCEPTRON_H

#include "core/IPerceptron.h"
#include <vector>
#include <cstddef>

namespace s21 {
namespace mlp {

class GraphPerceptron : public IPerceptron {
 public:
  explicit GraphPerceptron(const std::vector<size_t>& layer_sizes);
  ~GraphPerceptron() override = default;

  std::vector<double> Predict(const std::vector<double>& input) override;
  void Forward(const std::vector<double>& input) override;
  void Backward(const std::vector<double>& target) override;
  void UpdateWeights(double learning_rate) override;

  void SetWeights(const std::vector<std::vector<double>>& weights) override;
  std::vector<std::vector<double>> GetWeights() const override;
  size_t InputSize() const override { return input_size_; }
  size_t OutputSize() const override { return output_size_; }

  const std::vector<size_t>& LayerSizes() const { return layer_sizes_; }

 private:
  struct Neuron {
    double value = 0.0;
    double bias = 0.0;
    std::vector<double> input_weights;
    std::vector<double> weight_gradients;
    double bias_gradient = 0.0;
  };

  std::vector<size_t> layer_sizes_;
  size_t input_size_;
  size_t output_size_;
  std::vector<std::vector<Neuron>> layers_;   // индекс 0 – входной слой (не используется)
  std::vector<std::vector<double>> activations_;

  void InitializeWeights();
};

}  // namespace mlp
}  // namespace s21
#endif  // MLP_GRAPH_PERCEPTRON_H
