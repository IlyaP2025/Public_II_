#ifndef MLP_EMNIST_LOADER_H
#define MLP_EMNIST_LOADER_H

#include "core/IDataLoader.h"
#include <string>

namespace s21 {
namespace mlp {

// Загрузчик датасета EMNIST-letters
class EmnistLoader : public IDataLoader {
 public:
  // Загрузить CSV и разделить на train/test.
  // Параметры: путь к файлу, доля тестовой выборки (0 < test_ratio < 1).
  std::pair<Dataset, Dataset> Load(const std::string& path,
                                   double test_ratio) override;

  // Загрузить CSV как один датасет.
  Dataset LoadFile(const std::string& path) override;

  // Разделить готовый датасет на две части.
  std::pair<Dataset, Dataset> Split(const Dataset& data,
                                    double test_ratio) override;
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_EMNIST_LOADER_H
