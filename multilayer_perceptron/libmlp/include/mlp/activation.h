#ifndef MLP_ACTIVATION_H
#define MLP_ACTIVATION_H

#include <cmath>

namespace s21 {
namespace mlp {

// Сигмоидная функция активации
inline double sigmoid(double x) {
    return 1.0 / (1.0 + std::exp(-x));
}

// Производная сигмоиды, y = sigmoid(x)
inline double sigmoid_derivative(double y) {
    return y * (1.0 - y);
}

}  // namespace mlp
}  // namespace s21

#endif  // MLP_ACTIVATION_H
