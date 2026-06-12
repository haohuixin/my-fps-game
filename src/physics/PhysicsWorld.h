#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include <glm/vec3.hpp>

namespace fps::physics {

struct RaycastHit {
  std::size_t enemyId = 0;
  float distance = 0.0f;
  glm::vec3 position{};
};

class PhysicsWorld {
 public:
  bool initialize();
  void resetPlayer(const glm::vec3& feetPosition);
  void setEnemyCollider(std::size_t enemyId, const glm::vec3& position, float radius, bool active);
  void stepCharacter(const glm::vec3& desiredVelocity, bool jumpPressed, float dt);
  [[nodiscard]] std::optional<RaycastHit> raycastEnemies(const glm::vec3& origin, const glm::vec3& direction,
                                                         float maxDistance) const;
  [[nodiscard]] const glm::vec3& playerFeetPosition() const { return playerFeetPosition_; }
  [[nodiscard]] bool playerGrounded() const { return grounded_; }
  [[nodiscard]] bool hasJoltHook() const;

 private:
  struct EnemyCollider {
    std::size_t enemyId = 0;
    glm::vec3 position{};
    float radius = 0.6f;
    bool active = true;
  };

  glm::vec3 playerFeetPosition_{0.0f, 0.0f, 4.0f};
  glm::vec3 playerVelocity_{};
  std::vector<EnemyCollider> enemyColliders_;
  bool grounded_ = true;
};

}  // namespace fps::physics
