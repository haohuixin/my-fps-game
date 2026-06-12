#pragma once

#include <glm/vec3.hpp>

#include "core/InputState.h"

namespace fps::gameplay {

class FirstPersonCamera;

class PlayerController {
 public:
  void captureInput(const fps::core::InputState& input, const FirstPersonCamera& camera);
  [[nodiscard]] glm::vec3 desiredVelocity() const;
  [[nodiscard]] bool consumeJumpPressed();

 private:
  glm::vec3 desiredVelocity_{};
  bool jumpQueued_ = false;
  float walkSpeed_ = 5.0f;
};

}  // namespace fps::gameplay
