#ifndef MLP_SIMPLE_WEIGHTS_SERIALIZER_H
#define MLP_SIMPLE_WEIGHTS_SERIALIZER_H

#include "core/IWeightsSerializer.h"
#include <string>

namespace s21 {
namespace mlp {

class SimpleWeightsSerializer : public IWeightsSerializer {
public:
    void Save(const std::string& filename,
              const std::vector<std::vector<double>>& weights) override;
    std::vector<std::vector<double>> Load(const std::string& filename) override;
};

} // namespace mlp
} // namespace s21
#endif
