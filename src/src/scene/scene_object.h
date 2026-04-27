#ifndef S21_SCENE_OBJECT_H
#define S21_SCENE_OBJECT_H

#include <memory>
#include <string>
#include <vector>

#include "common/transform.h"

namespace s21 {

class SceneObject {
 public:
  SceneObject() = default;
  virtual ~SceneObject() = default;

  // Имя
  void SetName(const std::string& name) { name_ = name; }
  const std::string& GetName() const { return name_; }

  // Источник (файл)
  void SetSourceFile(const std::string& filename) { sourceFile_ = filename; }
  const std::string& GetSourceFile() const { return sourceFile_; }

  // Трансформация
  Transform& GetTransform() { return transform_; }
  const Transform& GetTransform() const { return transform_; }
  void SetTransform(const Transform& transform) { transform_ = transform; }

  // Иерархия
  void SetParent(SceneObject* parent);
  SceneObject* GetParent() const { return parent_; }

  void AddChild(std::unique_ptr<SceneObject> child);
  std::unique_ptr<SceneObject> RemoveChild(SceneObject* child);
  const std::vector<std::unique_ptr<SceneObject>>& GetChildren() const {
    return children_;
  }

  // Видимость и выделение
  void SetVisible(bool visible) { visible_ = visible; }
  bool IsVisible() const { return visible_; }

  void SetSelected(bool selected) { selected_ = selected; }
  bool IsSelected() const { return selected_; }

 protected:
  std::string name_;
  std::string sourceFile_;  // добавлено
  Transform transform_;
  SceneObject* parent_ = nullptr;
  std::vector<std::unique_ptr<SceneObject>> children_;
  bool visible_ = true;
  bool selected_ = false;
};

}  // namespace s21

#endif  // S21_SCENE_OBJECT_H