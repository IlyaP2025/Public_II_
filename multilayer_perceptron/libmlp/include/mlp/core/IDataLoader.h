#ifndef MLP_IDATALOADER_H
#define MLP_IDATALOADER_H

#include "ITrainer.h"
#include <utility>
#include <vector>
#include <string>

namespace s21 {
namespace mlp {

// Интерфейс загрузчика данных: чтение, нормализация, разделение.
class IDataLoader {
 public:
  virtual ~IDataLoader() = default;

  // Загрузить данные и разбить на обучающую и тестовую выборки в соотношении test_ratio.
  virtual std::pair<Dataset, Dataset> Load(const std::string& path, double test_ratio) = 0;

  // Загрузить файл как один полный набор данных.
  virtual Dataset LoadFile(const std::string& path) = 0;

  // Разделить переданный набор на две части (train / test).
  virtual std::pair<Dataset, Dataset> Split(const Dataset& data, double test_ratio) = 0;
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_IDATALOADER_H
