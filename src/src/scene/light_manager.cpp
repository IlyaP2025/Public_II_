#include "light_manager.h"
#include <algorithm>
#include <stdexcept>

namespace s21 {

size_t LightManager::AddLight(const LightSource& light) {
  if (lights_.size() >= kMaxLights) {
    throw std::runtime_error("Cannot add more than 5 light sources");
  }
  lights_.push_back(light);
  NotifyChange();
  return lights_.size() - 1;
}

void LightManager::RemoveLight(size_t index) {
  if (index >= lights_.size()) {
    throw std::out_of_range("Light index out of range");
  }
  lights_.erase(lights_.begin() + index);
  NotifyChange();
}

void LightManager::UpdateLight(size_t index, const LightSource& light) {
  if (index >= lights_.size()) {
    throw std::out_of_range("Light index out of range");
  }
  if (lights_[index] != light) {
    lights_[index] = light;
    NotifyChange();
  }
}

const LightSource& LightManager::GetLight(size_t index) const {
  if (index >= lights_.size()) {
    throw std::out_of_range("Light index out of range");
  }
  return lights_[index];
}

size_t LightManager::GetLightCount() const { return lights_.size(); }

std::vector<LightSource> LightManager::GetActiveLights() const {
  std::vector<LightSource> active;
  std::copy_if(lights_.begin(), lights_.end(), std::back_inserter(active),
               [](const LightSource& l) { return l.enabled; });
  return active;
}

void LightManager::Clear() {
  if (!lights_.empty()) {
    lights_.clear();
    NotifyChange();
  }
}

void LightManager::SetChangeCallback(std::function<void()> callback) {
  change_callback_ = std::move(callback);
}

void LightManager::NotifyChange() {
  if (change_callback_) {
    change_callback_();
  }
}

}  // namespace s21
