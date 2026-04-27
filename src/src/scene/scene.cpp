#include "scene.h"

#include <algorithm>

#include "common/debug.h"

namespace s21 {

SceneObject* Scene::AddObject(std::unique_ptr<SceneObject> object,
                              SceneObject* parent) {
  DEBUG_PRINT_FUNC();
  (void)parent;
  SceneObject* ptr = object.get();
  objects_.push_back(std::move(object));
  for (auto obs : observers_) {
    obs->OnObjectAdded(ptr);
  }
  return ptr;
}

void Scene::RemoveObject(SceneObject* object) {
  DEBUG_PRINT_FUNC();
  auto it = std::find_if(objects_.begin(), objects_.end(),
                         [object](const std::unique_ptr<SceneObject>& ptr) {
                           return ptr.get() == object;
                         });
  if (it == objects_.end()) return;

  // Если объект выделен, убираем его из списка выделенных до удаления
  auto selIt = std::find(selected_.begin(), selected_.end(), object);
  if (selIt != selected_.end()) {
    selected_.erase(selIt);
    // Уведомляем об изменении выделения
    for (auto obs : observers_) {
      obs->OnSelectionChanged(selected_);
    }
  }

  // Извлекаем объект из списка (теперь он хранится локально)
  std::unique_ptr<SceneObject> objHolder = std::move(*it);
  objects_.erase(it);

  // Уведомляем об удалении (объект ещё жив)
  for (auto obs : observers_) {
    obs->OnObjectRemoved(objHolder.get());
  }

  // При выходе из функции objHolder уничтожит объект
}

void Scene::NotifyTransformChanged(SceneObject* object) {
  for (auto obs : observers_) {
    obs->OnTransformChanged(object);
  }
}

void Scene::SetSelected(const std::vector<SceneObject*>& selected) {
  DEBUG_PRINT("Selected " << selected.size() << " objects");  // исправлено
  selected_ = selected;
  for (auto obs : observers_) {
    obs->OnSelectionChanged(selected_);
  }
}

void Scene::RemoveObserver(SceneObserver* observer) {
  auto it = std::find(observers_.begin(), observers_.end(), observer);
  if (it != observers_.end()) {
    observers_.erase(it);
  }
}

std::vector<const Mesh*> Scene::GetAllMeshes() const {
  std::vector<const Mesh*> result;
  for (const auto& obj : objects_) {
    if (auto* mesh = dynamic_cast<const Mesh*>(obj.get())) {
      result.push_back(mesh);
    }
  }
  return result;
}

Scene::Scene() {
  light_manager_.SetChangeCallback([this]() { NotifyLightsChanged(); });
}

void Scene::Clear() {
  // Сначала сбрасываем выделение, чтобы наблюдатели перестали ссылаться на
  // удаляемые объекты
  SetSelected({});

  // Затем удаляем все объекты (при удалении каждого будут уведомления, но
  // selected_ уже пуст)
  while (!objects_.empty()) {
    SceneObject* obj = objects_.back().get();
    RemoveObject(obj);  // внутри объекта уведомление и удаление
  }

  light_manager_.Clear();
  structureDirty_ = true;

}

void Scene::NotifyLightsChanged() {
  // уведомление наблюдателей (можно расширить SceneObserver)
  for (auto* obs : observers_) {
    obs->OnLightsChanged();  // если добавите метод в интерфейс
  }
}

std::vector<BoundingBox> Scene::GetMeshBoundingBoxes() const {
  std::vector<BoundingBox> boxes;
  for (const auto& obj : objects_) {
    if (auto* mesh = dynamic_cast<const Mesh*>(obj.get())) {
      BoundingBox local = mesh->GetBoundingBox();
      S21Matrix model = mesh->GetTransform().GetModelMatrix();
      // трансформация 8 вершин и вычисление мирового AABB
      // (реализация как в предоставленном примере)
      boxes.push_back(worldBox);
    }
  }
  return boxes;
}

void Scene::RebuildSpatialIndex() {
  auto boxes = GetMeshBoundingBoxes();
  spatialIndex_->Build(boxes);
  structureDirty_ = false;
}

}  // namespace s21
