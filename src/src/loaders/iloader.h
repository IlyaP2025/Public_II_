#ifndef S21_ILOADER_H
#define S21_ILOADER_H

#include <memory>
#include <string>
#include <vector>

namespace s21 {

class SceneObject;  // опережающее объявление

class ILoader {
 public:
  virtual ~ILoader() = default;

  // Загружает модель из файла filename, создаёт объекты сцены и помещает их в
  // outObjects. В случае ошибки возвращает false и заполняет error.
  virtual bool Load(const std::string& filename,
                    std::vector<std::unique_ptr<SceneObject>>& outObjects,
                    std::string& error) = 0;
};

}  // namespace s21

#endif  // S21_ILOADER_H