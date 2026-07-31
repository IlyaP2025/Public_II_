#ifndef MLP_MATRIX_PERCEPTRON_H
#define MLP_MATRIX_PERCEPTRON_H

#include <vector>
#include <cstddef>

#include "matrix/s21_matrix_oop.h"
#include "core/IPerceptron.h"

namespace s21 {
namespace mlp {

// Матричная реализация многослойного перцептрона
class MatrixPerceptron : public IPerceptron {
 public:
    // Конструктор: список размеров слоёв, включая входной и выходной
    explicit MatrixPerceptron(const std::vector<size_t>& layer_sizes);

    // Реализация интерфейса IPerceptron
    std::vector<double> Predict(const std::vector<double>& input) override;
    void SetWeights(const std::vector<std::vector<double>>& weights) override;
    std::vector<std::vector<double>> GetWeights() const override;
    size_t InputSize() const override;
    size_t OutputSize() const override;

    // Методы для обучения (вызываются тренером)
    void Forward(const std::vector<double>& input);
    void Backward(const std::vector<double>& target);
    void UpdateWeights(double learning_rate);

    // Доступ к размерам слоёв (для тестов)
    const std::vector<size_t>& LayerSizes() const { return layer_sizes_; }

 private:
    std::vector<size_t> layer_sizes_;
    std::vector<S21Matrix> weights_;      // матрицы весов между слоями
    std::vector<S21Matrix> biases_;       // векторы смещений (столбцы)
    std::vector<S21Matrix> activations_;  // выходы слоёв после сигмоиды
    std::vector<S21Matrix> zs_;           // взвешенные суммы до активации
    std::vector<S21Matrix> weight_gradients_;
    std::vector<S21Matrix> bias_gradients_;
    std::vector<double> last_output_;

    void InitializeWeights();  // случайная инициализация малыми значениями
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_MATRIX_PERCEPTRON_H
