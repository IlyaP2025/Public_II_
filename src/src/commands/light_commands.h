#ifndef S21_COMMANDS_LIGHT_COMMANDS_H_
#define S21_COMMANDS_LIGHT_COMMANDS_H_

#include "command.h"
#include "common/lighting.h"
#include "scene/scene.h"

namespace s21 {

class AddLightCommand : public Command {
 public:
  AddLightCommand(Scene* scene, const LightSource& light)
      : scene_(scene), light_(light) {
    if (!scene_) throw std::invalid_argument("Scene is null");
  }

  void execute() override {
    added_index_ = scene_->GetLightManager().AddLight(light_);
  }

  void undo() override {
    scene_->GetLightManager().RemoveLight(added_index_);
  }

 private:
  Scene* scene_;
  LightSource light_;
  size_t added_index_ = 0;
};

class RemoveLightCommand : public Command {
 public:
  RemoveLightCommand(Scene* scene, size_t index)
      : scene_(scene), index_(index) {
    if (!scene_) throw std::invalid_argument("Scene is null");
    light_ = scene_->GetLightManager().GetLight(index_);
  }

  void execute() override { scene_->GetLightManager().RemoveLight(index_); }

  void undo() override {
    scene_->GetLightManager().AddLight(light_);
  }

 private:
  Scene* scene_;
  size_t index_;
  LightSource light_;
};

class UpdateLightCommand : public Command {
 public:
  UpdateLightCommand(Scene* scene, size_t index, const LightSource& new_light)
      : scene_(scene), index_(index), new_light_(new_light) {
    if (!scene_) throw std::invalid_argument("Scene is null");
    old_light_ = scene_->GetLightManager().GetLight(index_);
  }

  void execute() override {
    scene_->GetLightManager().UpdateLight(index_, new_light_);
  }

  void undo() override {
    scene_->GetLightManager().UpdateLight(index_, old_light_);
  }

 private:
  Scene* scene_;
  size_t index_;
  LightSource new_light_;
  LightSource old_light_;
};

}  // namespace s21

#endif  // S21_COMMANDS_LIGHT_COMMANDS_H_
