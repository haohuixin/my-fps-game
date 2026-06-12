#include "gameplay/WeaponSystem.h"

namespace fps::gameplay {

std::optional<fps::physics::RaycastHit> WeaponSystem::fire(const glm::vec3& origin, const glm::vec3& direction,
                                                           const fps::physics::PhysicsWorld& physicsWorld) const {
  return physicsWorld.raycastEnemies(origin, direction, range_);
}

}  // namespace fps::gameplay
