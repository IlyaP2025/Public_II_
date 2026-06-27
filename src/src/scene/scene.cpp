#include <cfloat>
#include "scene.h"
#include "common/debug.h"
#include "common/transform.h"
#include "scene/ray_mesh_intersect.h"

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
  structureDirty_ = true;
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
  structureDirty_ = true;
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
  analyticObjects_.clear();
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

      std::vector<Point> corners = {{local.min.x, local.min.y, local.min.z},
                                    {local.max.x, local.min.y, local.min.z},
                                    {local.min.x, local.max.y, local.min.z},
                                    {local.max.x, local.max.y, local.min.z},
                                    {local.min.x, local.min.y, local.max.z},
                                    {local.max.x, local.min.y, local.max.z},
                                    {local.min.x, local.max.y, local.max.z},
                                    {local.max.x, local.max.y, local.max.z}};

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

// ===================== ИСПРАВЛЕННЫЙ МЕТОД =====================
bool Scene::TraceRay(const Ray& ray, float t_min, float t_max, HitRecord& rec) const {
    bool hitAnything = false;
    float closest = t_max;

    auto traceMesh = [&](const Mesh* mesh, const Ray& worldRay) -> bool {
        if (!mesh) return false;

        S21Matrix model = mesh->GetTransform().GetModelMatrix();
        S21Matrix invModel;
        try {
            invModel = model.InverseMatrix();
        } catch (...) {
            return false;
        }

        Point localOrigin = TransformPoint(invModel, worldRay.origin);
        Point localDir = TransformDirection(invModel, worldRay.direction);
        Ray localRay{localOrigin, localDir};

        HitRecord meshRec;
        if (RayMeshIntersect(*mesh, localRay, t_min, closest, meshRec)) {
            if (meshRec.t < closest) {
                closest = meshRec.t;
                rec.t = meshRec.t;

                rec.point = TransformPoint(model, meshRec.point);

                S21Matrix normalMatrix = model;
                normalMatrix(0,3) = normalMatrix(1,3) = normalMatrix(2,3) = 0.0;
                Point worldNormal = TransformDirection(normalMatrix, meshRec.normal);
                rec.normal = worldNormal.Normalize();

                rec.material = meshRec.material;
                return true;
            }
        }
        return false;
    };

    // Проверка мешей через KD-дерево
    if (spatialIndex_ && spatialIndex_->IsBuilt()) {
        std::vector<size_t> candidates = spatialIndex_->QueryRay(ray.origin, ray.direction);
        for (size_t idx : candidates) {
            if (idx >= objects_.size()) continue;
            const Mesh* mesh = dynamic_cast<const Mesh*>(objects_[idx].get());
            if (traceMesh(mesh, ray)) {
                hitAnything = true;
            }
        }
    } else {
        for (const auto& obj : objects_) {
            const Mesh* mesh = dynamic_cast<const Mesh*>(obj.get());
            if (traceMesh(mesh, ray)) {
                hitAnything = true;
            }
        }
    }

    return hitAnything;
}
// =============================================================

void Scene::AddAnalyticObject(std::unique_ptr<AnalyticObject> obj) {
    analyticObjects_.push_back(std::move(obj));
}

}  // namespace s21
