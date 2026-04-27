#include <iostream>

#include "facade.h"
#include "common/debug.h"
#include "commands/light_commands.h"
#include "loaders/iloader.h"
#include "loaders/loader_factory.h"

namespace s21 {

Facade::Facade(std::shared_ptr<Scene> scene)
    : scene_(scene), stateMachine_(std::make_unique<AppStateMachine>()) {}

void Facade::MoveSelected(const Point& delta) {
  DEBUG_PRINT_FUNC();
  if (!CanTransform()) {
    DEBUG_PRINT("MoveSelected blocked by state");
    return;
  }
  auto selected = scene_->GetSelected();
  if (selected.empty()) return;

  if (selected.size() == 1) {
    SceneObject* obj = selected.front();
    Point newPos = obj->GetTransform().GetPosition() + delta;
    auto cmd = std::make_unique<MoveCommand>(scene_.get(), obj, newPos);
    cmdManager_.executeCommand(std::move(cmd));
  } else {
    auto macro = std::make_unique<MacroCommand>();
    for (SceneObject* obj : selected) {
      Point newPos = obj->GetTransform().GetPosition() + delta;
      macro->addCommand(
          std::make_unique<MoveCommand>(scene_.get(), obj, newPos));
    }
    cmdManager_.executeCommand(std::move(macro));
  }
}

void Facade::RotateSelected(const Point& delta) {
  DEBUG_PRINT_FUNC();
  if (!CanTransform()) {
    DEBUG_PRINT("RotateSelected blocked by state");
    return;
  }
  auto selected = scene_->GetSelected();
  if (selected.empty()) return;

  if (selected.size() == 1) {
    SceneObject* obj = selected.front();
    Point newRot = obj->GetTransform().GetRotation() + delta;
    auto cmd = std::make_unique<RotateCommand>(scene_.get(), obj, newRot);
    cmdManager_.executeCommand(std::move(cmd));
  } else {
    auto macro = std::make_unique<MacroCommand>();
    for (SceneObject* obj : selected) {
      Point newRot = obj->GetTransform().GetRotation() + delta;
      macro->addCommand(
          std::make_unique<RotateCommand>(scene_.get(), obj, newRot));
    }
    cmdManager_.executeCommand(std::move(macro));
  }
}

void Facade::ScaleSelected(const Point& delta) {
  DEBUG_PRINT_FUNC();
  if (!CanTransform()) {
    DEBUG_PRINT("ScaleSelected blocked by state");
    return;
  }
  auto selected = scene_->GetSelected();
  if (selected.empty()) return;

  if (selected.size() == 1) {
    SceneObject* obj = selected.front();
    Point newScale = obj->GetTransform().GetScale() + delta;
    if (newScale.x <= 0) newScale.x = 0.1f;
    if (newScale.y <= 0) newScale.y = 0.1f;
    if (newScale.z <= 0) newScale.z = 0.1f;
    auto cmd = std::make_unique<ScaleCommand>(scene_.get(), obj, newScale);
    cmdManager_.executeCommand(std::move(cmd));
  } else {
    auto macro = std::make_unique<MacroCommand>();
    for (SceneObject* obj : selected) {
      Point newScale = obj->GetTransform().GetScale() + delta;
      if (newScale.x <= 0) newScale.x = 0.1f;
      if (newScale.y <= 0) newScale.y = 0.1f;
      if (newScale.z <= 0) newScale.z = 0.1f;
      macro->addCommand(
          std::make_unique<ScaleCommand>(scene_.get(), obj, newScale));
    }
    cmdManager_.executeCommand(std::move(macro));
  }
}

void Facade::Undo() {
  DEBUG_PRINT_FUNC();
  if (!CanTransform()) {
    DEBUG_PRINT("Undo blocked by state");
    return;
  }
  cmdManager_.undo();
}

void Facade::Redo() {
  DEBUG_PRINT_FUNC();
  if (!CanTransform()) {
    DEBUG_PRINT("Redo blocked by state");
    return;
  }
  cmdManager_.redo();
}

bool Facade::CanUndo() const { return cmdManager_.canUndo(); }

bool Facade::CanRedo() const { return cmdManager_.canRedo(); }

void Facade::LoadModel() {
  // Не используется, оставлен для совместимости
  if (!CanLoadModel()) return;
}

// ===== Методы для конечного автомата =====
bool Facade::CanLoadModel() const {
  bool result = stateMachine_->CanLoadModel();
  DEBUG_PRINT("CanLoadModel: " << result << " state="
                               << static_cast<int>(stateMachine_->GetState()));
  return result;
}

bool Facade::CanTransform() const {
  bool result = stateMachine_->CanTransform();
  DEBUG_PRINT("CanTransform: " << result << " state="
                               << static_cast<int>(stateMachine_->GetState()));
  return result;
}

bool Facade::CanStartGifRecording() const {
  return stateMachine_->CanStartGifRecording();
}

bool Facade::CanStopGifRecording() const {
  return stateMachine_->CanStopGifRecording();
}

bool Facade::CanSaveScreenshot() const {
  return stateMachine_->CanSaveScreenshot();
}

bool Facade::CanResetCamera() const { return stateMachine_->CanResetCamera(); }

std::string Facade::GetErrorMessage() const {
  return stateMachine_->GetErrorMessage();
}

void Facade::ResetError() { stateMachine_->OnResetError(); }

void Facade::NotifyLoadStarted() {
  DEBUG_PRINT_FUNC();
  if (!CanLoadModel()) {
    DEBUG_PRINT("NotifyLoadStarted: blocked by state");
    return;
  }
  stateMachine_->OnLoadStarted();
}

void Facade::NotifyLoadFinished(bool success, const std::string& errorMessage) {
  DEBUG_PRINT_FUNC();
  // Проверяем, что находимся в состоянии Loading, иначе игнорируем
  if (stateMachine_->GetState() != AppState::Loading) {
    DEBUG_PRINT("NotifyLoadFinished: not in Loading state, ignored");
    return;
  }
  stateMachine_->OnLoadFinished(success, errorMessage);
}

void Facade::NotifyGifRecordingStarted() {
  if (!CanStartGifRecording()) return;
  stateMachine_->OnStartGifRecording();
}

void Facade::NotifyGifRecordingFinished() {
  if (!CanStopGifRecording()) return;
  stateMachine_->OnStopGifRecording();
}

void Facade::NotifyModelCleared() {
  DEBUG_PRINT_FUNC();
  stateMachine_->OnModelCleared();
}

void Facade::AddStateObserver(AppStateMachine::Observer observer) {
  stateMachine_->AddObserver(observer);
}

AppState Facade::GetState() const { return stateMachine_->GetState(); }

// Методы для освещения
void Facade::AddLight(const LightSource& light) {
  auto cmd = std::make_unique<AddLightCommand>(scene_.get(), light);
  cmdManager_.executeCommand(std::move(cmd));
}

void Facade::RemoveLight(size_t index) {
  auto cmd = std::make_unique<RemoveLightCommand>(scene_.get(), index);
  cmdManager_.executeCommand(std::move(cmd));
}

void Facade::UpdateLight(size_t index, const LightSource& light) {
  auto cmd = std::make_unique<UpdateLightCommand>(scene_.get(), index, light);
  cmdManager_.executeCommand(std::move(cmd));
}

std::vector<LightSource> Facade::GetLights() const {
  std::vector<LightSource> all;
  for (size_t i = 0; i < scene_->GetLightManager().GetLightCount(); ++i)
    all.push_back(scene_->GetLightManager().GetLight(i));
  return all;
}

}  // namespace s21
