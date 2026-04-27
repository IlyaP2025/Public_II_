#ifndef S21_COMMON_LIGHTING_H_
#define S21_COMMON_LIGHTING_H_

#include <glm/glm.hpp>
#include <vector>
#include <functional>

namespace s21 {

struct LightSource {
  enum class Type { kPoint = 0 };

  Type type = Type::kPoint;
  glm::vec3 position{0.0f, 0.0f, 0.0f};
  glm::vec3 ambient{0.1f, 0.1f, 0.1f};
  glm::vec3 diffuse{1.0f, 1.0f, 1.0f};
  glm::vec3 specular{1.0f, 1.0f, 1.0f};
  bool enabled = true;

  bool operator==(const LightSource& other) const {
    return type == other.type && position == other.position &&
           ambient == other.ambient && diffuse == other.diffuse &&
           specular == other.specular && enabled == other.enabled;
  }
  bool operator!=(const LightSource& other) const { return !(*this == other); }
};

class ILightManager {
 public:
  virtual ~ILightManager() = default;
  virtual size_t AddLight(const LightSource& light) = 0;
  virtual void RemoveLight(size_t index) = 0;
  virtual void UpdateLight(size_t index, const LightSource& light) = 0;
  virtual const LightSource& GetLight(size_t index) const = 0;
  virtual size_t GetLightCount() const = 0;
  virtual std::vector<LightSource> GetActiveLights() const = 0;
  virtual void Clear() = 0;
  virtual void SetChangeCallback(std::function<void()> callback) = 0;
};

}  // namespace s21

#endif  // S21_COMMON_LIGHTING_H_
