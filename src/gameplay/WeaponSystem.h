#pragma once

#include <cstddef>
#include <optional>

#include <glm/vec3.hpp>

#include "physics/PhysicsWorld.h"

namespace fps::gameplay {

class WeaponSystem {
 public:
  [[nodiscard]] std::optional<fps::physics::RaycastHit> fire(const glm::vec3& origin, const glm::vec3& direction,
                                                             const fps::physics::PhysicsWorld& physicsWorld) const;

 private:
  float range_ = 40.0f;
};

}  // namespace fps::gameplay
