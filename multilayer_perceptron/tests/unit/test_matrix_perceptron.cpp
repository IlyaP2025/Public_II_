#include <gtest/gtest.h>
#include "perceptron/matrix_perceptron.h"
#include "trainer/simple_trainer.h"
#include <random>
#include <cmath>

using namespace s21::mlp;

TEST(MatrixPerceptronTest, ForwardOutputSize) {
    MatrixPerceptron net({2, 3, 1});
    std::vector<double> input = {0.5, -0.2};
    auto output = net.Predict(input);
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

TEST(MatrixPerceptronTest, InputOutputSizes) {
    MatrixPerceptron net({4, 6, 3});
    EXPECT_EQ(net.InputSize(), 4u);
    EXPECT_EQ(net.OutputSize(), 3u);
    const auto& layers = net.LayerSizes();
    ASSERT_EQ(layers.size(), 3u);
    EXPECT_EQ(layers[0], 4u);
    EXPECT_EQ(layers[1], 6u);
    EXPECT_EQ(layers[2], 3u);
}

TEST(MatrixPerceptronTest, SetGetWeights) {
    MatrixPerceptron net({3, 5, 2});
    auto weights = net.GetWeights();
    ASSERT_EQ(weights.size(), 2u);

    // Проверяем размеры: первый слой (5x3) + 5 смещений = 20, второй слой (2x5) + 2 = 12
    EXPECT_EQ(weights[0].size(), 5u * 3u + 5u);
    EXPECT_EQ(weights[1].size(), 2u * 5u + 2u);

    // Увеличиваем все веса в 2 раза
    for (auto& layer : weights) {
        for (auto& v : layer) v *= 2.0;
    }
    net.SetWeights(weights);

    // Проверяем, что предсказание не падает
    std::vector<double> input = {0.1, 0.2, 0.3};
    auto output = net.Predict(input);
    ASSERT_EQ(output.size(), 2u);
}

TEST(MatrixPerceptronTest, CrossValidateSmoke) {
    // Маленький датасет из 20 примеров с двумя классами (для скорости)
    MatrixPerceptron prototype({2, 3, 2});
    Dataset data;
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    for (int i = 0; i < 20; ++i) {
        std::vector<double> input = {dis(gen), dis(gen)};
        std::vector<double> target = (input[0] > 0) ? std::vector<double>{1.0, 0.0}
                                                    : std::vector<double>{0.0, 1.0};
        data.emplace_back(std::move(input), std::move(target));
    }

    SimpleTrainer trainer(0.1, 1, true);
    double acc = trainer.CrossValidate(prototype, data, 2);
    EXPECT_GE(acc, 0.0);
    EXPECT_LE(acc, 1.0);
}
