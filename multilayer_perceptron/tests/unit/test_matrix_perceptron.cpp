#include <gtest/gtest.h>
#include "perceptron/matrix_perceptron.h"
#include <cmath>

using namespace s21::mlp;

TEST(MatrixPerceptronTest, ForwardOutputSize) {
    MatrixPerceptron net({2, 3, 1});
    std::vector<double> input = {0.5, -0.2};
    auto output = net.Forward(input);
    ASSERT_EQ(output.size(), 1u);
}

TEST(MatrixPerceptronTest, TrainXOR) {
    MatrixPerceptron net({2, 4, 1});

    std::vector<std::pair<std::vector<double>, std::vector<double>>> dataset = {
        {{0.0, 0.0}, {0.0}},
        {{0.0, 1.0}, {1.0}},
        {{1.0, 0.0}, {1.0}},
        {{1.0, 1.0}, {0.0}}
    };

    double learning_rate = 0.5;
    int epochs = 5000;

    for (int epoch = 0; epoch < epochs; ++epoch) {
        for (const auto& sample : dataset) {
            net.Forward(sample.first);
            net.Backward(sample.second);
            net.UpdateWeights(learning_rate);
        }
    }

    double total_error = 0.0;
    for (const auto& sample : dataset) {
        auto out = net.Predict(sample.first);
        double err = out[0] - sample.second[0];
        total_error += err * err;
    }
    total_error /= dataset.size();
    EXPECT_LT(total_error, 0.01);
}
