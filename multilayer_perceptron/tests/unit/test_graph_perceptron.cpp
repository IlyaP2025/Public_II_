#include <gtest/gtest.h>
#include "perceptron/graph_perceptron.h"
#include "activation.h"
#include <cmath>

using namespace s21::mlp;

TEST(GraphPerceptronTest, ConstructorAndSizes) {
    GraphPerceptron net({3, 4, 2});
    EXPECT_EQ(net.InputSize(), 3u);
    EXPECT_EQ(net.OutputSize(), 2u);
    const auto& layers = net.LayerSizes();
    ASSERT_EQ(layers.size(), 3u);
    EXPECT_EQ(layers[0], 3u);
    EXPECT_EQ(layers[1], 4u);
    EXPECT_EQ(layers[2], 2u);
}

TEST(GraphPerceptronTest, ForwardOutputSize) {
    GraphPerceptron net({2, 5, 1});
    std::vector<double> input = {0.5, -0.2};
    auto output = net.Predict(input);
    ASSERT_EQ(output.size(), 1u);
}

TEST(GraphPerceptronTest, SetAndGetWeights) {
    GraphPerceptron net({2, 3, 1});

    // Получаем начальные веса
    auto weights = net.GetWeights();
    ASSERT_EQ(weights.size(), 2u);   // два слоя (скрытый и выходной)

    // Умножаем все веса на 2 и смещения тоже на 2
    for (auto& layer : weights) {
        for (auto& v : layer) {
            v *= 2.0;
        }
    }

    net.SetWeights(weights);

    // Проверяем, что после загрузки предсказание изменилось (или хотя бы не падает)
    std::vector<double> input = {0.1, 0.9};
    auto output = net.Predict(input);
    ASSERT_EQ(output.size(), 1u);
}

TEST(GraphPerceptronTest, TrainXOR) {
    GraphPerceptron net({2, 8, 1});   // скрытый слой 8 нейронов

    std::vector<std::pair<std::vector<double>, std::vector<double>>> dataset = {
        {{0.0, 0.0}, {0.0}},
        {{0.0, 1.0}, {1.0}},
        {{1.0, 0.0}, {1.0}},
        {{1.0, 1.0}, {0.0}}
    };

    double learning_rate = 0.7;
    int epochs = 30000;

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
    EXPECT_LT(total_error, 0.05);
}
