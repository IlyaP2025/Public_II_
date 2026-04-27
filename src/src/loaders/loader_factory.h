#ifndef S21_LOADER_FACTORY_H
#define S21_LOADER_FACTORY_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "iloader.h"

namespace s21 {

class LoaderFactory {
 public:
  using Creator = std::function<std::unique_ptr<ILoader>()>;

  static LoaderFactory& Instance() {
    static LoaderFactory instance;
    return instance;
  }

  // Только объявления методов
  void Register(const std::string& extension, Creator creator);
  std::unique_ptr<ILoader> CreateLoader(const std::string& extension) const;

 private:
  LoaderFactory() = default;
  std::unordered_map<std::string, Creator> creators_;
};

}  // namespace s21

#endif  // S21_LOADER_FACTORY_H