#include "gameplay/PlayerController.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/geometric.hpp>

#include "gameplay/FirstPersonCamera.h"

namespace fps::gameplay {

void PlayerController::captureInput(const fps::core::InputState& input, const FirstPersonCamera& camera) {
  glm::vec3 movement{};
  if (input.keys[GLFW_KEY_W]) {
    movement += camera.planarForward();
  }
  if (input.keys[GLFW_KEY_S]) {
    movement -= camera.planarForward();
  }
  if (input.keys[GLFW_KEY_A]) {
    movement -= camera.planarRight();
  }
  if (input.keys[GLFW_KEY_D]) {
    movement += camera.planarRight();
  }

  if (glm::length(movement) > 0.001f) {
    movement = glm::normalize(movement) * walkSpeed_;
  }
  desiredVelocity_ = movement;
  jumpQueued_ = jumpQueued_ || input.jumpPressed;
}

glm::vec3 PlayerController::desiredVelocity() const {
  return desiredVelocity_;
}

bool PlayerController::consumeJumpPressed() {
  const bool result = jumpQueued_;
  jumpQueued_ = false;
  return result;
}

}  // namespace fps::gameplay
