#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace fps::gameplay {

class FirstPersonCamera {
 public:
  void bindToPlayerFeet(const glm::vec3& playerFeetPosition);
  void applyMouseDelta(float deltaX, float deltaY);
  [[nodiscard]] glm::vec3 front() const;
  [[nodiscard]] glm::vec3 planarForward() const;
  [[nodiscard]] glm::vec3 planarRight() const;
  [[nodiscard]] glm::vec3 eyePosition() const;
  [[nodiscard]] glm::mat4 viewMatrix() const;

 private:
  glm::vec3 playerFeetPosition_{0.0f, 0.0f, 0.0f};
  float yaw_ = -90.0f;
  float pitch_ = 0.0f;
  float eyeHeight_ = 1.65f;
  float sensitivity_ = 0.1f;
};

}  // namespace fps::gameplay
