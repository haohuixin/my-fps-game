#include "gameplay/FirstPersonCamera.h"

#include <algorithm>

#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

namespace fps::gameplay {

void FirstPersonCamera::bindToPlayerFeet(const glm::vec3& playerFeetPosition) {
  playerFeetPosition_ = playerFeetPosition;
}

void FirstPersonCamera::applyMouseDelta(float deltaX, float deltaY) {
  yaw_ += deltaX * sensitivity_;
  pitch_ += deltaY * sensitivity_;
  pitch_ = std::clamp(pitch_, -89.0f, 89.0f);
}

glm::vec3 FirstPersonCamera::front() const {
  glm::vec3 direction{};
  direction.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
  direction.y = std::sin(glm::radians(pitch_));
  direction.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
  return glm::normalize(direction);
}

glm::vec3 FirstPersonCamera::planarForward() const {
  glm::vec3 forward = front();
  forward.y = 0.0f;
  if (glm::length(forward) <= 0.001f) {
    return {0.0f, 0.0f, -1.0f};
  }
  return glm::normalize(forward);
}

glm::vec3 FirstPersonCamera::planarRight() const {
  return glm::normalize(glm::cross(planarForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
}

glm::vec3 FirstPersonCamera::eyePosition() const {
  return playerFeetPosition_ + glm::vec3(0.0f, eyeHeight_, 0.0f);
}

glm::mat4 FirstPersonCamera::viewMatrix() const {
  const glm::vec3 eye = eyePosition();
  return glm::lookAt(eye, eye + front(), glm::vec3(0.0f, 1.0f, 0.0f));
}

}  // namespace fps::gameplay
