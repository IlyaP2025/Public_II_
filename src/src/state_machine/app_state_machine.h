#ifndef APP_STATE_MACHINE_H
#define APP_STATE_MACHINE_H

#include <QMutex>
#include <functional>
#include <string>
#include <vector>

enum class AppState { Idle, Loading, Ready, RecordingGif, Error };

class AppStateMachine {
 public:
  using Observer = std::function<void(AppState new_state)>;

  bool CanLoadModel() const;
  bool CanCancelLoad() const;
  bool CanTransform() const;
  bool CanChangeSettings() const;
  bool CanStartGifRecording() const;
  bool CanStopGifRecording() const;
  bool CanSaveScreenshot() const;
  bool CanResetCamera() const;

  void OnLoadStarted();
  void OnLoadFinished(bool success, const std::string& error_message = "");
  void OnStartGifRecording();
  void OnStopGifRecording();
  void OnError(const std::string& message);
  void OnResetError();
  void OnModelCleared();

  void AddObserver(Observer observer);
  AppState GetState() const;
  std::string GetErrorMessage() const;

 private:
  void TransitionTo(AppState new_state);
  bool IsValidTransition(AppState from, AppState to) const;
  void NotifyObservers();

  mutable QMutex mutex_;
  AppState state_ = AppState::Idle;
  std::string error_message_;
  std::vector<Observer> observers_;
};

#endif  // APP_STATE_MACHINE_H
