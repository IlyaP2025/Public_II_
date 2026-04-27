#ifndef S21_SCENE_OBSERVER_H
#define S21_SCENE_OBSERVER_H

#include <vector>
#include "scene_object.h"

namespace s21 {

class SceneObserver {
 public:
  virtual ~SceneObserver() = default;

  virtual void OnObjectAdded(SceneObject* object) = 0;
  virtual void OnObjectRemoved(SceneObject* object) = 0;
  virtual void OnSelectionChanged(
      const std::vector<SceneObject*>& selected) = 0;
  virtual void OnTransformChanged(SceneObject* object) = 0;
  
  virtual void OnLightsChanged() {}
};

}  // namespace s21

#endif  // S21_SCENE_OBSERVER_H
