#include "loader_factory.h"

#include <utility>  // для std::move

namespace s21 {

void LoaderFactory::Register(const std::string& extension, Creator creator) {
  creators_[extension] = std::move(creator);
}

std::unique_ptr<ILoader> LoaderFactory::CreateLoader(
    const std::string& extension) const {
  auto it = creators_.find(extension);
  if (it != creators_.end()) {
    return it->second();
  }
  return nullptr;
}

}  // namespace s21