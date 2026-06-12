#include "ui/HudLayer.h"

#include <array>
#include <string>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "rendering/OpenGLCompat.h"

namespace fps::ui {

bool HudLayer::initialize(GLFWwindow* window) {
  window_ = window;
  syncWindowTitle();
  return window_ != nullptr;
}

void HudLayer::setScore(int score) {
  score_ = score;
  syncWindowTitle();
}

void HudLayer::setRuntimeFlags(bool hasJoltHook, bool hasEffekseerHook, bool hasNanoguiHook) {
  hasJoltHook_ = hasJoltHook;
  hasEffekseerHook_ = hasEffekseerHook;
  hasNanoguiHook_ = hasNanoguiHook;
  syncWindowTitle();
}

void HudLayer::draw(int framebufferWidth, int framebufferHeight) const {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, framebufferWidth, framebufferHeight, 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);

  glColor3f(0.95f, 0.95f, 0.95f);
  glLineWidth(2.0f);
  glBegin(GL_LINES);
  glVertex2f(framebufferWidth * 0.5f - 10.0f, framebufferHeight * 0.5f);
  glVertex2f(framebufferWidth * 0.5f + 10.0f, framebufferHeight * 0.5f);
  glVertex2f(framebufferWidth * 0.5f, framebufferHeight * 0.5f - 10.0f);
  glVertex2f(framebufferWidth * 0.5f, framebufferHeight * 0.5f + 10.0f);
  glEnd();

  std::string scoreText = std::to_string(score_);
  float cursorX = 22.0f;
  for (char digit : scoreText) {
    drawDigit(cursorX, 22.0f, 14.0f, digit - '0');
    cursorX += 18.0f;
  }

  glEnable(GL_DEPTH_TEST);
}

void HudLayer::syncWindowTitle() const {
  if (window_ == nullptr) {
    return;
  }
  std::string title = "my-fps-game | score=" + std::to_string(score_);
  title += hasJoltHook_ ? " | Jolt hook" : " | fallback physics";
  title += hasEffekseerHook_ ? " | Effekseer hook" : " | builtin hit fx";
  title += hasNanoguiHook_ ? " | NanoGUI hook" : " | builtin HUD";
  glfwSetWindowTitle(window_, title.c_str());
}

void HudLayer::drawDigit(float x, float y, float scale, int digit) const {
  static constexpr std::array<std::array<bool, 7>, 10> kSegments{{
      {{true, true, true, false, true, true, true}},
      {{false, false, true, false, false, true, false}},
      {{true, false, true, true, true, false, true}},
      {{true, false, true, true, false, true, true}},
      {{false, true, true, true, false, true, false}},
      {{true, true, false, true, false, true, true}},
      {{true, true, false, true, true, true, true}},
      {{true, false, true, false, false, true, false}},
      {{true, true, true, true, true, true, true}},
      {{true, true, true, true, false, true, true}},
  }};

  if (digit < 0 || digit > 9) {
    return;
  }

  glColor3f(0.95f, 0.95f, 0.95f);
  const auto& segments = kSegments[digit];
  if (segments[0]) drawSegment(x, y, x + scale, y);
  if (segments[1]) drawSegment(x, y, x, y + scale);
  if (segments[2]) drawSegment(x + scale, y, x + scale, y + scale);
  if (segments[3]) drawSegment(x, y + scale, x + scale, y + scale);
  if (segments[4]) drawSegment(x, y + scale, x, y + scale * 2.0f);
  if (segments[5]) drawSegment(x + scale, y + scale, x + scale, y + scale * 2.0f);
  if (segments[6]) drawSegment(x, y + scale * 2.0f, x + scale, y + scale * 2.0f);
}

void HudLayer::drawSegment(float x1, float y1, float x2, float y2) const {
  glBegin(GL_LINES);
  glVertex2f(x1, y1);
  glVertex2f(x2, y2);
  glEnd();
}

}  // namespace fps::ui
