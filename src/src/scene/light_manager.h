#ifndef S21_SCENE_LIGHT_MANAGER_H_
#define S21_SCENE_LIGHT_MANAGER_H_

#include <functional>
#include <vector>

#include "common/lighting.h"

namespace s21 {

class LightManager : public ILightManager {
 public:
  LightManager() = default;
  ~LightManager() override = default;

  LightManager(const LightManager&) = delete;
  LightManager& operator=(const LightManager&) = delete;
  LightManager(LightManager&&) = delete;
  LightManager& operator=(LightManager&&) = delete;

  size_t AddLight(const LightSource& light) override;
  void RemoveLight(size_t index) override;
  void UpdateLight(size_t index, const LightSource& light) override;
  const LightSource& GetLight(size_t index) const override;
  size_t GetLightCount() const override;
  std::vector<LightSource> GetActiveLights() const override;
  void Clear() override;
  void SetChangeCallback(std::function<void()> callback) override;

  void SetDirectionalLight(const DirectionalLight& light) override;
  const DirectionalLight& GetDirectionalLight() const override;

 private:
  static constexpr size_t kMaxLights = 5;
  std::vector<LightSource> lights_;
  std::function<void()> change_callback_;
  void NotifyChange();
  DirectionalLight dirLight_;
};

}  // namespace s21

#endif  // S21_SCENE_LIGHT_MANAGER_H_
