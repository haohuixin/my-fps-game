#pragma once

#include <cstddef>

#include <glm/vec3.hpp>

#include "rendering/SpriteAnimation.h"

namespace fps::physics {
class PhysicsWorld;
}

namespace fps::gameplay {

class EnemyActor {
 public:
  EnemyActor(std::size_t id, const glm::vec3& position, float radius,
             const fps::rendering::SpriteAnimationClip* idleClip);

  void update(float dt, fps::physics::PhysicsWorld& physicsWorld);
  [[nodiscard]] bool applyDamage(int damage);
  [[nodiscard]] bool alive() const { return alive_; }
  [[nodiscard]] std::size_t id() const { return id_; }
  [[nodiscard]] const glm::vec3& position() const { return position_; }
  [[nodiscard]] float radius() const { return radius_; }
  [[nodiscard]] const fps::rendering::SpriteFrame& currentFrame() const;

 private:
  std::size_t id_ = 0;
  glm::vec3 position_{};
  float radius_ = 0.6f;
  int health_ = 1;
  bool alive_ = true;
  fps::rendering::SpriteAnimator animator_;
};

}  // namespace fps::gameplay
