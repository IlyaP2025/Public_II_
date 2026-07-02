#include "data/emnist_loader.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <random>
#include <algorithm>

namespace s21 {
namespace mlp {

std::pair<Dataset, Dataset> EmnistLoader::Load(const std::string& path,
                                               double test_ratio) {
  Dataset full = LoadFile(path);
  return Split(full, test_ratio);
}

Dataset EmnistLoader::LoadFile(const std::string& path) {
  Dataset data;
  std::ifstream file(path);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open file: " + path);
  }

  std::string line;
  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string cell;

    // Чтение метки
    if (!std::getline(ss, cell, ',')) continue;
    int label = std::stoi(cell);
    // EMNIST-letters: метки 1..26, приводим к 0..25
    int class_idx = label - 1;
    if (class_idx < 0 || class_idx > 25) continue;  // пропускаем некорректные

    // Чтение пикселей
    std::vector<double> input(784);
    for (int i = 0; i < 784; ++i) {
      if (!std::getline(ss, cell, ',')) break;
      input[i] = std::stod(cell) / 255.0;  // нормализация [0..255] -> [0..1]
    }

    // One-hot вектор цели
    std::vector<double> target(26, 0.0);
    target[class_idx] = 1.0;

    data.emplace_back(std::move(input), std::move(target));
  }
  return data;
}

std::pair<Dataset, Dataset> EmnistLoader::Split(const Dataset& data,
                                                double test_ratio) {
  if (test_ratio <= 0.0 || test_ratio >= 1.0) {
    throw std::invalid_argument("test_ratio must be in (0, 1)");
  }
  // Копируем данные и перемешиваем
  Dataset shuffled = data;
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(shuffled.begin(), shuffled.end(), g);

  size_t test_size = static_cast<size_t>(shuffled.size() * test_ratio);
  Dataset test(shuffled.begin(), shuffled.begin() + test_size);
  Dataset train(shuffled.begin() + test_size, shuffled.end());
  return {train, test};
}

}  // namespace mlp
}  // namespace s21
