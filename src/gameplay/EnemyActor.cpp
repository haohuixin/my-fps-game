#include "gameplay/EnemyActor.h"

#include "physics/PhysicsWorld.h"

namespace fps::gameplay {

EnemyActor::EnemyActor(std::size_t id, const glm::vec3& position, float radius,
                       const fps::rendering::SpriteAnimationClip* idleClip)
    : id_(id), position_(position), radius_(radius) {
  animator_.play(idleClip);
}

void EnemyActor::update(float dt, fps::physics::PhysicsWorld& physicsWorld) {
  animator_.update(dt);
  physicsWorld.setEnemyCollider(id_, position_, radius_, alive_);
}

bool EnemyActor::applyDamage(int damage) {
  if (!alive_) {
    return false;
  }
  health_ -= damage;
  if (health_ <= 0) {
    alive_ = false;
    return true;
  }
  return false;
}

const fps::rendering::SpriteFrame& EnemyActor::currentFrame() const {
  return animator_.currentFrame();
}

}  // namespace fps::gameplay
