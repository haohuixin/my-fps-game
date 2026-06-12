#pragma once

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace fps::rendering {

class ParticleSystem {
 public:
  bool initialize();
  void triggerHit(const glm::vec3& position);
  void update(float dt);
  void draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPosition) const;
  [[nodiscard]] bool hasEffekseerHook() const;

 private:
  struct Particle {
    glm::vec3 position{};
    float age = 0.0f;
    float lifetime = 0.35f;
    float size = 0.15f;
  };

  std::vector<Particle> particles_;
};

}  // namespace fps::rendering
