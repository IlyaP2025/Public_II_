#ifndef S21_TRANSFORM_COMMANDS_H
#define S21_TRANSFORM_COMMANDS_H

#include <memory>
#include <stdexcept>

#include "command.h"
#include "common/point.h"
#include "scene/scene.h"
#include "scene/scene_object.h"

namespace s21 {

class TransformCommand : public Command {
 public:
  TransformCommand(Scene* scene, SceneObject* obj)
      : scene_(scene), object_(obj) {
    if (!scene || !obj)
      throw std::invalid_argument("TransformCommand: null pointer");
  }
  ~TransformCommand() override = default;

 protected:
  Scene* scene_;
  SceneObject* object_;
};

class MoveCommand : public TransformCommand {
 public:
  MoveCommand(Scene* scene, SceneObject* obj, const Point& newPos)
      : TransformCommand(scene, obj), newPos_(newPos) {
    oldPos_ = obj->GetTransform().GetPosition();
  }

  void execute() override {
    object_->GetTransform().SetPosition(newPos_);
    scene_->NotifyTransformChanged(object_);
  }

  void undo() override {
    object_->GetTransform().SetPosition(oldPos_);
    scene_->NotifyTransformChanged(object_);
  }

 private:
  Point oldPos_;
  Point newPos_;
};

class RotateCommand : public TransformCommand {
 public:
  RotateCommand(Scene* scene, SceneObject* obj, const Point& newRot)
      : TransformCommand(scene, obj), newRot_(newRot) {
    oldRot_ = obj->GetTransform().GetRotation();
  }

  void execute() override {
    object_->GetTransform().SetRotation(newRot_);
    scene_->NotifyTransformChanged(object_);
  }

  void undo() override {
    object_->GetTransform().SetRotation(oldRot_);
    scene_->NotifyTransformChanged(object_);
  }

 private:
  Point oldRot_;
  Point newRot_;
};

class ScaleCommand : public TransformCommand {
 public:
  ScaleCommand(Scene* scene, SceneObject* obj, const Point& newScale)
      : TransformCommand(scene, obj), newScale_(newScale) {
    oldScale_ = obj->GetTransform().GetScale();
  }

  void execute() override {
    object_->GetTransform().SetScale(newScale_);
    scene_->NotifyTransformChanged(object_);
  }

  void undo() override {
    object_->GetTransform().SetScale(oldScale_);
    scene_->NotifyTransformChanged(object_);
  }

 private:
  Point oldScale_;
  Point newScale_;
};

}  // namespace s21

#endif  // S21_TRANSFORM_COMMANDS_H
