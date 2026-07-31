#include "perceptron/graph_perceptron.h"
#include "activation.h"
#include <random>
#include <stdexcept>
#include <algorithm>

namespace s21 {
namespace mlp {

GraphPerceptron::GraphPerceptron(const std::vector<size_t>& layer_sizes)
    : layer_sizes_(layer_sizes),
      input_size_(layer_sizes.front()),
      output_size_(layer_sizes.back()) {
  if (layer_sizes_.size() < 2) {
    throw std::invalid_argument("At least 2 layers required");
  }
  InitializeWeights();
}

void GraphPerceptron::InitializeWeights() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(-0.5, 0.5);

  layers_.resize(layer_sizes_.size());
  for (size_t i = 1; i < layer_sizes_.size(); ++i) {
    size_t num_neurons = layer_sizes_[i];
    size_t prev_size = layer_sizes_[i - 1];
    layers_[i].resize(num_neurons);
    for (auto& neuron : layers_[i]) {
      neuron.input_weights.resize(prev_size);
      neuron.weight_gradients.resize(prev_size, 0.0);
      for (auto& w : neuron.input_weights) w = dis(gen);
      neuron.bias = dis(gen);
    }
  }
}

std::vector<double> GraphPerceptron::Predict(const std::vector<double>& input) {
  Forward(input);
  return activations_.back();
}

void GraphPerceptron::Forward(const std::vector<double>& input) {
  if (input.size() != input_size_) {
    throw std::invalid_argument("Input size mismatch");
  }

  activations_.resize(layer_sizes_.size());
  activations_[0] = input;

  for (size_t layer_idx = 1; layer_idx < layer_sizes_.size(); ++layer_idx) {
    const auto& prev = activations_[layer_idx - 1];
    auto& cur_layer = layers_[layer_idx];
    activations_[layer_idx].resize(cur_layer.size());
    for (size_t n = 0; n < cur_layer.size(); ++n) {
      double sum = cur_layer[n].bias;
      for (size_t w = 0; w < cur_layer[n].input_weights.size(); ++w) {
        sum += cur_layer[n].input_weights[w] * prev[w];
      }
      double val = sigmoid(sum);
      cur_layer[n].value = val;
      activations_[layer_idx][n] = val;
    }
  }
}

void GraphPerceptron::Backward(const std::vector<double>& target) {
  size_t last = layer_sizes_.size() - 1;
  std::vector<double> deltas(layer_sizes_[last]);

  // Выходной слой
  for (size_t i = 0; i < deltas.size(); ++i) {
    double o = layers_[last][i].value;
    deltas[i] = (o - target[i]) * o * (1.0 - o);
  }

  // Скрытые слои
  for (int l = static_cast<int>(last) - 1; l >= 1; --l) {
    std::vector<double> new_deltas(layer_sizes_[l], 0.0);
    for (size_t j = 0; j < layers_[l].size(); ++j) {
      double err = 0.0;
      for (size_t k = 0; k < layers_[l + 1].size(); ++k) {
        err += layers_[l + 1][k].input_weights[j] * deltas[k];
      }
      new_deltas[j] = err * layers_[l][j].value * (1.0 - layers_[l][j].value);
    }
    deltas.swap(new_deltas);
  }

  // Накапливаем градиенты
  for (size_t l = 1; l <= last; ++l) {
    const auto& prev_act = activations_[l - 1];
    auto& cur_layer = layers_[l];
    for (size_t n = 0; n < cur_layer.size(); ++n) {
      double d = deltas[n];
      for (size_t w = 0; w < cur_layer[n].input_weights.size(); ++w) {
        cur_layer[n].weight_gradients[w] += d * prev_act[w];
      }
      cur_layer[n].bias_gradient += d;
    }
  }
}

void GraphPerceptron::UpdateWeights(double learning_rate) {
  for (size_t l = 1; l < layers_.size(); ++l) {
    for (auto& neuron : layers_[l]) {
      for (size_t w = 0; w < neuron.input_weights.size(); ++w) {
        neuron.input_weights[w] -= learning_rate * neuron.weight_gradients[w];
        neuron.weight_gradients[w] = 0.0;
      }
      neuron.bias -= learning_rate * neuron.bias_gradient;
      neuron.bias_gradient = 0.0;
    }
  }
}

// Заглушки сериализации
void GraphPerceptron::SetWeights(const std::vector<std::vector<double>>&) {
  throw std::logic_error("SetWeights not implemented for GraphPerceptron");
}

std::vector<std::vector<double>> GraphPerceptron::GetWeights() const {
  throw std::logic_error("GetWeights not implemented for GraphPerceptron");
}

}  // namespace mlp
}  // namespace s21
