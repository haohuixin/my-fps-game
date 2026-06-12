#pragma once

#include <array>

namespace fps::core {

struct InputState {
  std::array<bool, 1024> keys{};
  bool jumpPressed = false;
  bool shootPressed = false;
  bool mouseInitialized = false;
  float mouseDeltaX = 0.0f;
  float mouseDeltaY = 0.0f;
  float lastMouseX = 0.0f;
  float lastMouseY = 0.0f;
};

}  // namespace fps::core
