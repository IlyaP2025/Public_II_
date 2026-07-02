#include "perceptron/matrix_perceptron.h"
#include "activation.h"

#include <stdexcept>
#include <random>
#include <cassert>

namespace s21 {
namespace mlp {

MatrixPerceptron::MatrixPerceptron(const std::vector<size_t>& layer_sizes)
    : layer_sizes_(layer_sizes) {
    if (layer_sizes_.size() < 2) {
        throw std::invalid_argument(
            "Perceptron must have at least 2 layers (input and output).");
    }
    InitializeWeights();
}

void MatrixPerceptron::InitializeWeights() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-0.5, 0.5);

    size_t num_layers = layer_sizes_.size();
    weights_.resize(num_layers - 1);
    biases_.resize(num_layers - 1);

    for (size_t i = 0; i < num_layers - 1; ++i) {
        int rows = static_cast<int>(layer_sizes_[i + 1]);
        int cols = static_cast<int>(layer_sizes_[i]);
        S21Matrix w(rows, cols);
        S21Matrix b(rows, 1);
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                w(r, c) = dis(gen);
            }
            b(r, 0) = dis(gen);
        }
        weights_[i] = w;
        biases_[i] = b;
    }
}

std::vector<double> MatrixPerceptron::Predict(const std::vector<double>& input) {
    return Forward(input);
}

std::vector<double> MatrixPerceptron::Forward(const std::vector<double>& input) {
    if (input.size() != layer_sizes_[0]) {
        throw std::invalid_argument("Input size doesn't match first layer size.");
    }

    S21Matrix current(static_cast<int>(input.size()), 1);
    for (size_t i = 0; i < input.size(); ++i) {
        current(static_cast<int>(i), 0) = input[i];
    }

    activations_.clear();
    zs_.clear();
    activations_.push_back(current);  // входной «слой» (без активации)

    for (size_t i = 0; i < weights_.size(); ++i) {
        S21Matrix z = weights_[i] * current;
        for (int r = 0; r < z.get_rows(); ++r) {
            z(r, 0) += biases_[i](r, 0);
        }
        zs_.push_back(z);

        S21Matrix a = z;
        a.Apply(sigmoid);
        activations_.push_back(a);
        current = a;
    }

    const S21Matrix& output = activations_.back();
    std::vector<double> result(output.get_rows());
    for (int i = 0; i < output.get_rows(); ++i) {
        result[i] = output(i, 0);
    }
    return result;
}

void MatrixPerceptron::Backward(const std::vector<double>& target) {
    size_t num_layers = weights_.size();
    weight_gradients_.resize(num_layers);
    bias_gradients_.resize(num_layers);
    for (size_t i = 0; i < num_layers; ++i) {
        weight_gradients_[i] = S21Matrix(weights_[i].get_rows(), weights_[i].get_cols());
        bias_gradients_[i] = S21Matrix(biases_[i].get_rows(), 1);
    }

    // Ошибка выходного слоя: delta = (a - target) * sigmoid'(a)
    S21Matrix& output_act = activations_.back();
    S21Matrix delta(output_act.get_rows(), 1);
    for (int r = 0; r < output_act.get_rows(); ++r) {
        double a = output_act(r, 0);
        double t = target[static_cast<size_t>(r)];
        delta(r, 0) = (a - t) * sigmoid_derivative(a);
    }

    // Градиенты для выходного слоя
    S21Matrix& prev_act = activations_[activations_.size() - 2];
    for (int r = 0; r < delta.get_rows(); ++r) {
        for (int c = 0; c < prev_act.get_rows(); ++c) {
            weight_gradients_.back()(r, c) += delta(r, 0) * prev_act(c, 0);
        }
        bias_gradients_.back()(r, 0) += delta(r, 0);
    }

    // Обратное распространение через скрытые слои
    for (int layer = static_cast<int>(num_layers) - 2; layer >= 0; --layer) {
        S21Matrix& w_next = weights_[layer + 1];
        S21Matrix& act_curr = activations_[layer + 1];
        S21Matrix delta_curr(act_curr.get_rows(), 1);

        for (int r = 0; r < act_curr.get_rows(); ++r) {
            double sum = 0.0;
            for (int k = 0; k < delta.get_rows(); ++k) {
                sum += w_next(k, r) * delta(k, 0);
            }
            delta_curr(r, 0) = sum * sigmoid_derivative(act_curr(r, 0));
        }

        S21Matrix& prev_a = activations_[layer];
        for (int r = 0; r < delta_curr.get_rows(); ++r) {
            for (int c = 0; c < prev_a.get_rows(); ++c) {
                weight_gradients_[layer](r, c) += delta_curr(r, 0) * prev_a(c, 0);
            }
            bias_gradients_[layer](r, 0) += delta_curr(r, 0);
        }

        delta = delta_curr;
    }
}

void MatrixPerceptron::UpdateWeights(double learning_rate) {
    for (size_t i = 0; i < weights_.size(); ++i) {
        S21Matrix gradient_scaled = weight_gradients_[i];
        gradient_scaled.MulNumber(learning_rate);
        weights_[i].SubMatrix(gradient_scaled);

        S21Matrix bias_scaled = bias_gradients_[i];
        bias_scaled.MulNumber(learning_rate);
        biases_[i].SubMatrix(bias_scaled);
    }
}

void MatrixPerceptron::SetWeights(const std::vector<std::vector<double>>&) {
    throw std::logic_error("SetWeights not implemented yet");
}

std::vector<std::vector<double>> MatrixPerceptron::GetWeights() const {
    throw std::logic_error("GetWeights not implemented yet");
}

size_t MatrixPerceptron::InputSize() const {
    return layer_sizes_.front();
}

size_t MatrixPerceptron::OutputSize() const {
    return layer_sizes_.back();
}

}  // namespace mlp
}  // namespace s21
