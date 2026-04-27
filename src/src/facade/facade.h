#ifndef S21_FACADE_H
#define S21_FACADE_H

#include <memory>
#include <string>
#include <vector>

#include "commands/command_manager.h"
#include "commands/macro_command.h"
#include "commands/transform_commands.h"
#include "common/point.h"
#include "scene/scene.h"
#include "state_machine/app_state_machine.h"

namespace s21 {

class Facade {
 public:
  explicit Facade(std::shared_ptr<Scene> scene);
  ~Facade() = default;

  // Действия над выделенными объектами
  void MoveSelected(const Point& delta);
  void RotateSelected(const Point& delta);
  void ScaleSelected(const Point& delta);

  // Undo/Redo
  void Undo();
  void Redo();
  bool CanUndo() const;
  bool CanRedo() const;

  // Загрузка модели
  void LoadModel();

  // Доступ к выделенным объектам
  std::vector<SceneObject*> GetSelected() const {
    return scene_->GetSelected();
  }
  std::shared_ptr<Scene> GetScene() const { return scene_; }

  // ===== Методы для конечного автомата =====
  bool CanLoadModel() const;
  bool CanTransform() const;
  bool CanStartGifRecording() const;
  bool CanStopGifRecording() const;
  bool CanSaveScreenshot() const;
  bool CanResetCamera() const;
  std::string GetErrorMessage() const;
  void ResetError();

  void NotifyLoadStarted();
  void NotifyLoadFinished(bool success, const std::string& errorMessage = "");
  void NotifyGifRecordingStarted();
  void NotifyGifRecordingFinished();
  void NotifyModelCleared();  // добавлено

  void AddStateObserver(AppStateMachine::Observer observer);
  AppState GetState() const;  // добавлено

 private:
  std::shared_ptr<Scene> scene_;
  CommandManager cmdManager_;
  std::unique_ptr<AppStateMachine> stateMachine_;
};

}  // namespace s21

#endif  // S21_FACADE_H
