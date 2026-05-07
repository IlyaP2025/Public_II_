#ifndef S21_SCENE_H
#define S21_SCENE_H

#include <algorithm>
#include <memory>
#include <vector>

#include "common/lighting.h"
#include "common/spatial_index.h"
#include "mesh.h"
#include "scene/kd_tree_index.h"
#include "scene/light_manager.h"
#include "scene_object.h"
#include "scene_observer.h"
#include "objects/analytic_object.h"

namespace s21 {

class Scene {
 public:
  Scene();
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

  ILightManager& GetLightManager() { return light_manager_; }
  const ILightManager& GetLightManager() const { return light_manager_; }

  std::vector<BoundingBox> GetMeshBoundingBoxes() const;
  void MarkStructureDirty() { structureDirty_ = true; }
  bool IsStructureDirty() const { return structureDirty_; }
  void ClearStructureDirty() { structureDirty_ = false; }
  void RebuildSpatialIndex();
  ISpatialIndex* GetSpatialIndex() { return spatialIndex_.get(); }

  void AddAnalyticObject(std::unique_ptr<AnalyticObject> obj) {
        analyticObjects_.push_back(std::move(obj));
  }  

  // Простая трассировка одного луча (без учёта освещения, только пересечение)
  bool TraceRay(const Ray& ray, float t_min, float t_max, HitRecord& rec) const;

 private:
  std::vector<std::unique_ptr<SceneObject>> objects_;
  std::vector<SceneObject*> selected_;
  std::vector<SceneObserver*> observers_;

  LightManager light_manager_;
  bool structureDirty_ = true;
  std::unique_ptr<ISpatialIndex> spatialIndex_ =
      std::make_unique<KdTreeMeshIndex>();

  void NotifyLightsChanged();

  std::vector<std::unique_ptr<AnalyticObject>> analyticObjects_;  

};

}  // namespace s21

#endif  // S21_SCENE_H
