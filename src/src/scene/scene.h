#ifndef S21_SCENE_H
#define S21_SCENE_H

#include <algorithm>
#include <memory>
#include <vector>

#include "mesh.h"
#include "scene_object.h"
#include "scene_observer.h"

namespace s21 {

class Scene {
 public:
  Scene() = default;
  ~Scene() = default;

  // Добавление объекта (теперь возвращает указатель на добавленный объект)
  SceneObject* AddObject(std::unique_ptr<SceneObject> object,
                         SceneObject* parent = nullptr);

  void RemoveObject(SceneObject* object);
  void NotifyTransformChanged(SceneObject* object);
  void SetSelected(const std::vector<SceneObject*>& selected);
  void AddObserver(SceneObserver* observer) { observers_.push_back(observer); }
  void RemoveObserver(SceneObserver* observer);
  void Clear();
  const std::vector<SceneObject*>& GetSelected() const { return selected_; }
  const std::vector<std::unique_ptr<SceneObject>>& GetObjects() const {
    return objects_;
  }
  std::vector<const Mesh*> GetAllMeshes() const;

 private:
  std::vector<std::unique_ptr<SceneObject>> objects_;
  std::vector<SceneObject*> selected_;
  std::vector<SceneObserver*> observers_;
};

}  // namespace s21

#endif  // S21_SCENE_H