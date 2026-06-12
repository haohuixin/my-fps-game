#include "physics/PhysicsWorld.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <glm/geometric.hpp>

#if FPS_HAS_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#endif

namespace fps::physics {

bool PhysicsWorld::initialize() {
  return true;
}

void PhysicsWorld::resetPlayer(const glm::vec3& feetPosition) {
  playerFeetPosition_ = feetPosition;
  playerVelocity_ = {};
  grounded_ = true;
}

void PhysicsWorld::setEnemyCollider(std::size_t enemyId, const glm::vec3& position, float radius, bool active) {
  const auto it = std::find_if(enemyColliders_.begin(), enemyColliders_.end(), [enemyId](const EnemyCollider& c) {
    return c.enemyId == enemyId;
  });
  if (it == enemyColliders_.end()) {
    enemyColliders_.push_back(EnemyCollider{enemyId, position, radius, active});
    return;
  }
  it->position = position;
  it->radius = radius;
  it->active = active;
}

void PhysicsWorld::stepCharacter(const glm::vec3& desiredVelocity, bool jumpPressed, float dt) {
  playerVelocity_.x = desiredVelocity.x;
  playerVelocity_.z = desiredVelocity.z;
  playerVelocity_.y += -18.0f * dt;

  if (grounded_ && jumpPressed) {
    playerVelocity_.y = 7.0f;
    grounded_ = false;
  }

  playerFeetPosition_ += playerVelocity_ * dt;
  if (playerFeetPosition_.y <= 0.0f) {
    playerFeetPosition_.y = 0.0f;
    playerVelocity_.y = 0.0f;
    grounded_ = true;
  }
}

std::optional<RaycastHit> PhysicsWorld::raycastEnemies(const glm::vec3& origin, const glm::vec3& direction,
                                                       float maxDistance) const {
  std::optional<RaycastHit> closestHit;
  float closestDistance = std::numeric_limits<float>::max();

  for (const auto& collider : enemyColliders_) {
    if (!collider.active) {
      continue;
    }

    const glm::vec3 oc = origin - collider.position;
    const float a = glm::dot(direction, direction);
    const float b = 2.0f * glm::dot(oc, direction);
    const float c = glm::dot(oc, oc) - collider.radius * collider.radius;
    const float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0.0f) {
      continue;
    }

    const float sqrtDisc = std::sqrt(discriminant);
    const float t1 = (-b - sqrtDisc) / (2.0f * a);
    const float t2 = (-b + sqrtDisc) / (2.0f * a);
    const float distance = t1 >= 0.0f ? t1 : t2;
    if (distance < 0.0f || distance > maxDistance || distance >= closestDistance) {
      continue;
    }

    closestDistance = distance;
    closestHit = RaycastHit{collider.enemyId, distance, origin + direction * distance};
  }

  return closestHit;
}

bool PhysicsWorld::hasJoltHook() const {
#if FPS_HAS_JOLT
  return true;
#else
  return false;
#endif
}

}  // namespace fps::physics
