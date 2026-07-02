#ifndef MLP_IWEIGHTSSERIALIZER_H
#define MLP_IWEIGHTSSERIALIZER_H

#include <string>
#include <vector>

namespace s21 {
namespace mlp {

// Интерфейс сохранения и загрузки весов перцептрона.
class IWeightsSerializer {
 public:
  virtual ~IWeightsSerializer() = default;

  // Сохранить веса в файл.
  virtual void Save(const std::string& filename, const std::vector<std::vector<double>>& weights) = 0;

  // Загрузить веса из файла.
  virtual std::vector<std::vector<double>> Load(const std::string& filename) = 0;
};

}  // namespace mlp
}  // namespace s21

#endif  // MLP_IWEIGHTSSERIALIZER_H
