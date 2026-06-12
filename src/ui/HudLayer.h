#pragma once

struct GLFWwindow;

namespace fps::ui {

class HudLayer {
 public:
  bool initialize(GLFWwindow* window);
  void setScore(int score);
  void setRuntimeFlags(bool hasJoltHook, bool hasEffekseerHook, bool hasNanoguiHook);
  void draw(int framebufferWidth, int framebufferHeight) const;

 private:
  void syncWindowTitle() const;
  void drawDigit(float x, float y, float scale, int digit) const;
  void drawSegment(float x1, float y1, float x2, float y2) const;

  GLFWwindow* window_ = nullptr;
  int score_ = 0;
  bool hasJoltHook_ = false;
  bool hasEffekseerHook_ = false;
  bool hasNanoguiHook_ = false;
};

}  // namespace fps::ui
