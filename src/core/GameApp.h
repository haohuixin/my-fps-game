#pragma once

#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "audio/AudioSystem.h"
#include "core/InputState.h"
#include "gameplay/EnemyActor.h"
#include "gameplay/FirstPersonCamera.h"
#include "gameplay/PlayerController.h"
#include "gameplay/WeaponSystem.h"
#include "physics/PhysicsWorld.h"
#include "rendering/ParticleSystem.h"
#include "rendering/SpriteAnimation.h"
#include "rendering/Texture2D.h"
#include "ui/HudLayer.h"

namespace fps::core {

class GameApp {
 public:
  bool initialize();
  int run();
  ~GameApp();

 private:
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
  static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
  static void cursorCallback(GLFWwindow* window, double x, double y);

  void onKey(int key, int action);
  void onMouseButton(int button, int action);
  void onCursor(double x, double y);

  void updateFrame(float frameDt);
  void fixedUpdate(float stepDt);
  void handleShot();
  void drawScene() const;
  void drawGround() const;
  void drawSpawnMarkers() const;
  void drawEnemy(const fps::gameplay::EnemyActor& enemy) const;

  GLFWwindow* window_ = nullptr;
  bool headlessMode_ = false;
  fps::core::InputState input_{};
  fps::gameplay::FirstPersonCamera camera_{};
  fps::gameplay::PlayerController playerController_{};
  fps::gameplay::WeaponSystem weaponSystem_{};
  fps::physics::PhysicsWorld physicsWorld_{};
  fps::rendering::Texture2D enemyTexture_{};
  fps::rendering::SpriteAnimationSet animationSet_{};
  fps::rendering::ParticleSystem particleSystem_{};
  fps::audio::AudioSystem audioSystem_{};
  fps::ui::HudLayer hudLayer_{};
  std::vector<fps::gameplay::EnemyActor> enemies_;
  int score_ = 0;
};

}  // namespace fps::core
