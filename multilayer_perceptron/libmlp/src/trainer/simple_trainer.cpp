#include "trainer/simple_trainer.h"
#include "perceptron/matrix_perceptron.h"
#include "perceptron/graph_perceptron.h"

#include <memory>
#include <random>
#include <algorithm>
#include <cassert>

namespace s21 {
namespace mlp {

void SimpleTrainer::Train(IPerceptron& perceptron,
                          const Dataset& train_data,
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
            perceptron.Forward(sample.first);
            perceptron.Backward(sample.second);
            perceptron.UpdateWeights(learning_rate_);
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

double SimpleTrainer::CrossValidate(IPerceptron& prototype, const Dataset& data, int k_folds) {
    if (k_folds < 2) throw std::invalid_argument("k_folds must be >= 2");
    if (data.empty()) throw std::invalid_argument("Dataset is empty");

    // Копируем и перемешиваем данные
    Dataset shuffled = data;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffled.begin(), shuffled.end(), g);

    size_t total = shuffled.size();
    size_t fold_size = total / k_folds;
    double total_accuracy = 0.0;

    for (int fold = 0; fold < k_folds; ++fold) {
        size_t start = fold * fold_size;
        size_t end = (fold == k_folds - 1) ? total : start + fold_size;

        // Разделяем на обучающую и тестовую части
        Dataset train, test;
        for (size_t i = 0; i < total; ++i) {
            if (i >= start && i < end) {
                test.push_back(shuffled[i]);
            } else {
                train.push_back(shuffled[i]);
            }
        }

        // Создаём свежую копию перцептрона той же архитектуры
        auto layers = prototype.LayerSizes();
        std::unique_ptr<IPerceptron> perceptron;
        // Определяем тип по имени класса (можно было бы использовать фабрику, но так проще)
        if (dynamic_cast<MatrixPerceptron*>(&prototype)) {
            perceptron = std::make_unique<MatrixPerceptron>(layers);
        } else {
            perceptron = std::make_unique<GraphPerceptron>(layers);
        }

        // Обучаем на этом фолде
        SimpleTrainer trainer(learning_rate_, epochs_, shuffle_);
        // Колбэк пустой, чтобы не спамить в лог
        auto empty_callback = [](int, double, double) {};
        trainer.Train(*perceptron, train, test, empty_callback);

        // Подсчитываем Accuracy на тестовой части
        int correct = 0;
        for (const auto& sample : test) {
            auto out = perceptron->Predict(sample.first);
            int pred = std::max_element(out.begin(), out.end()) - out.begin();
            int act = std::max_element(sample.second.begin(), sample.second.end()) - sample.second.begin();
            if (pred == act) ++correct;
        }
        double acc = static_cast<double>(correct) / test.size();
        total_accuracy += acc;
    }

    return total_accuracy / k_folds;
}

}  // namespace mlp
}  // namespace s21
