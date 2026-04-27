#ifndef S21_MODEL_BUILDER_H
#define S21_MODEL_BUILDER_H

#include <memory>
#include <string>

#include "common/model_data.h"
#include "mesh.h"

namespace s21 {

class ModelBuilder {
 public:
  static std::unique_ptr<Mesh> BuildFromRawData(const RawModelData& data);
};

}  // namespace s21

#endif  // S21_MODEL_BUILDER_H
