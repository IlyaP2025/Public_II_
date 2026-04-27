#include "scene.h"
#include "common/debug.h"
#include "common/transform.h"
#include <cfloat>

namespace s21 {

Scene::Scene() {
  light_manager_.SetChangeCallback([this]() { NotifyLightsChanged(); });
}

SceneObject* Scene::AddObject(std::unique_ptr<SceneObject> object,
                              SceneObject* parent) {
  DEBUG_PRINT_FUNC();
  (void)parent;
  SceneObject* ptr = object.get();
  objects_.push_back(std::move(object));
  for (auto obs : observers_) {
    obs->OnObjectAdded(ptr);
  }
  structureDirty_ = true;  // структура сцены изменилась
  return ptr;
}

void Scene::RemoveObject(SceneObject* object) {
  DEBUG_PRINT_FUNC();
  auto it = std::find_if(objects_.begin(), objects_.end(),
                         [object](const std::unique_ptr<SceneObject>& ptr) {
                           return ptr.get() == object;
                         });
  if (it == objects_.end()) return;

  auto selIt = std::find(selected_.begin(), selected_.end(), object);
  if (selIt != selected_.end()) {
    selected_.erase(selIt);
    for (auto obs : observers_) {
      obs->OnSelectionChanged(selected_);
    }
  }

  std::unique_ptr<SceneObject> objHolder = std::move(*it);
  objects_.erase(it);

  for (auto obs : observers_) {
    obs->OnObjectRemoved(objHolder.get());
  }
  structureDirty_ = true;
}

void Scene::NotifyTransformChanged(SceneObject* object) {
  for (auto obs : observers_) {
    obs->OnTransformChanged(object);
  }
  structureDirty_ = true; // трансформация могла изменить AABB
}

void Scene::SetSelected(const std::vector<SceneObject*>& selected) {
  DEBUG_PRINT("Selected " << selected.size() << " objects");
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

void Scene::Clear() {
  SetSelected({});
  while (!objects_.empty()) {
    SceneObject* obj = objects_.back().get();
    RemoveObject(obj);
  }
  light_manager_.Clear();
  structureDirty_ = true;
}

void Scene::NotifyLightsChanged() {
  for (auto* obs : observers_) {
    obs->OnLightsChanged();
  }
}

std::vector<BoundingBox> Scene::GetMeshBoundingBoxes() const {
  std::vector<BoundingBox> boxes;
  for (const auto& obj : objects_) {
    if (auto* mesh = dynamic_cast<const Mesh*>(obj.get())) {
      BoundingBox local = mesh->GetBoundingBox();
      S21Matrix model = mesh->GetTransform().GetModelMatrix();

      std::vector<Point> corners = {
          {local.min.x, local.min.y, local.min.z},
          {local.max.x, local.min.y, local.min.z},
          {local.min.x, local.max.y, local.min.z},
          {local.max.x, local.max.y, local.min.z},
          {local.min.x, local.min.y, local.max.z},
          {local.max.x, local.min.y, local.max.z},
          {local.min.x, local.max.y, local.max.z},
          {local.max.x, local.max.y, local.max.z}
      };

      Point worldMin(FLT_MAX, FLT_MAX, FLT_MAX);
      Point worldMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

      for (const auto& corner : corners) {
        Point transformed = TransformPoint(model, corner);
        worldMin.x = std::min(worldMin.x, transformed.x);
        worldMin.y = std::min(worldMin.y, transformed.y);
        worldMin.z = std::min(worldMin.z, transformed.z);
        worldMax.x = std::max(worldMax.x, transformed.x);
        worldMax.y = std::max(worldMax.y, transformed.y);
        worldMax.z = std::max(worldMax.z, transformed.z);
      }

      boxes.push_back({worldMin, worldMax});
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
