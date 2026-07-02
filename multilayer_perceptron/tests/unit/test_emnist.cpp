#include <gtest/gtest.h>
#include "perceptron/matrix_perceptron.h"
#include "data/emnist_loader.h"
#include "trainer/simple_trainer.h"
#include <iostream>
#include <cmath>
#include <fstream>

using namespace s21::mlp;

TEST(EmnistTest, LoadAndTrain) {
  // Проверяем, что файл существует
  std::string path = "../datasets/emnist-letters-train.csv";
  std::ifstream test_file(path);
  if (!test_file.is_open()) {
    GTEST_SKIP() << "Dataset not found: " << path;
  }
  test_file.close();

  EmnistLoader loader;
  auto [train, test] = loader.Load(path, 0.2);
  ASSERT_GT(train.size(), 0);
  ASSERT_GT(test.size(), 0);

  // Небольшая сеть для быстрого теста
  MatrixPerceptron net({784, 128, 26});
  SimpleTrainer trainer(0.1, 2);  // всего 2 эпохи для проверки работы

  double final_train_loss = 0.0;
  double final_valid_loss = 0.0;

  trainer.Train(net, train, test,
                [&](int epoch, double train_loss, double valid_loss) {
                  final_train_loss = train_loss;
                  final_valid_loss = valid_loss;
                  std::cout << "Epoch " << epoch
                            << " train_loss: " << train_loss
                            << " valid_loss: " << valid_loss << std::endl;
                });

  // Ошибка после 2 эпох должна быть конечной (не NaN)
  EXPECT_GT(final_train_loss, 0.0);
  EXPECT_LT(final_train_loss, 1.0);
  EXPECT_GT(final_valid_loss, 0.0);
  EXPECT_LT(final_valid_loss, 1.0);
}
