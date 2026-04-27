#include "app_state_machine.h"

#include <QMutexLocker>
#include <algorithm>

#include "common/debug.h"

bool AppStateMachine::CanLoadModel() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::Idle;
}

bool AppStateMachine::CanCancelLoad() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::Loading;
}

bool AppStateMachine::CanTransform() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::Ready;
}

bool AppStateMachine::CanChangeSettings() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::Ready;
}

bool AppStateMachine::CanStartGifRecording() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::Ready;
}

bool AppStateMachine::CanStopGifRecording() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::RecordingGif;
}

bool AppStateMachine::CanSaveScreenshot() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::Ready;
}

bool AppStateMachine::CanResetCamera() const {
  QMutexLocker locker(&mutex_);
  return state_ == AppState::Ready;
}

void AppStateMachine::OnLoadStarted() {
  QMutexLocker locker(&mutex_);
  TransitionTo(AppState::Loading);
}

void AppStateMachine::OnLoadFinished(bool success,
                                     const std::string& error_message) {
  QMutexLocker locker(&mutex_);
  if (success) {
    TransitionTo(AppState::Ready);
  } else {
    error_message_ = error_message;
    TransitionTo(AppState::Error);
  }
}

void AppStateMachine::OnStartGifRecording() {
  QMutexLocker locker(&mutex_);
  if (state_ == AppState::Ready) {
    TransitionTo(AppState::RecordingGif);
  }
}

void AppStateMachine::OnStopGifRecording() {
  QMutexLocker locker(&mutex_);
  if (state_ == AppState::RecordingGif) {
    TransitionTo(AppState::Ready);
  }
}

void AppStateMachine::OnError(const std::string& message) {
  QMutexLocker locker(&mutex_);
  error_message_ = message;
  TransitionTo(AppState::Error);
}

void AppStateMachine::OnResetError() {
  QMutexLocker locker(&mutex_);
  if (state_ == AppState::Error) {
    TransitionTo(AppState::Idle);
    error_message_.clear();
  }
}

void AppStateMachine::OnModelCleared() {
  QMutexLocker locker(&mutex_);
  if (state_ == AppState::Ready) {
    TransitionTo(AppState::Idle);
  }
}

void AppStateMachine::TransitionTo(AppState new_state) {
  if (!IsValidTransition(state_, new_state)) {
    DEBUG_PRINT("Invalid transition from " << static_cast<int>(state_) << " to "
                                           << static_cast<int>(new_state));
    return;
  }
  DEBUG_PRINT("State transition: " << static_cast<int>(state_) << " -> "
                                   << static_cast<int>(new_state));
  state_ = new_state;
  NotifyObservers();
}

bool AppStateMachine::IsValidTransition(AppState from, AppState to) const {
  switch (from) {
    case AppState::Idle:
      return to == AppState::Loading || to == AppState::Error;
    case AppState::Loading:
      return to == AppState::Ready || to == AppState::Error;
    case AppState::Ready:
      return to == AppState::RecordingGif || to == AppState::Error ||
             to == AppState::Idle || to == AppState::Loading;
    case AppState::RecordingGif:
      return to == AppState::Ready || to == AppState::Error;
    case AppState::Error:
      return to == AppState::Idle;
    default:
      return false;
  }
}

void AppStateMachine::NotifyObservers() {
  // mutex_ already locked by caller
  for (const auto& obs : observers_) {
    obs(state_);
  }
}

void AppStateMachine::AddObserver(Observer observer) {
  QMutexLocker locker(&mutex_);
  observers_.push_back(std::move(observer));
}

AppState AppStateMachine::GetState() const {
  QMutexLocker locker(&mutex_);
  return state_;
}

std::string AppStateMachine::GetErrorMessage() const {
  QMutexLocker locker(&mutex_);
  return error_message_;
}
