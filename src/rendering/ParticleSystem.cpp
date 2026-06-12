#include "rendering/ParticleSystem.h"

#include <algorithm>

#include "rendering/OpenGLCompat.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/trigonometric.hpp>

#if FPS_HAS_EFFEKSEER
#include <Effekseer.h>
#endif

namespace fps::rendering {

bool ParticleSystem::initialize() {
  return true;
}

void ParticleSystem::triggerHit(const glm::vec3& position) {
  particles_.push_back(Particle{position});
#if FPS_HAS_EFFEKSEER
  // Effekseer runtime can be bound here to emit a hit burst at `position`.
#endif
}

void ParticleSystem::update(float dt) {
  for (auto& particle : particles_) {
    particle.age += dt;
    particle.size += dt * 0.6f;
  }
  particles_.erase(std::remove_if(particles_.begin(), particles_.end(), [](const Particle& particle) {
                     return particle.age >= particle.lifetime;
                   }),
                   particles_.end());
}

void ParticleSystem::draw(const glm::mat4& view, const glm::mat4& projection,
                          const glm::vec3& cameraPosition) const {
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glDisable(GL_TEXTURE_2D);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(glm::value_ptr(projection));
  glMatrixMode(GL_MODELVIEW);

  for (const auto& particle : particles_) {
    const float alpha = 1.0f - particle.age / particle.lifetime;
    const glm::vec3 toCamera = glm::normalize(glm::vec3(cameraPosition.x - particle.position.x, 0.0f,
                                                        cameraPosition.z - particle.position.z));
    const float yaw = std::atan2(toCamera.x, toCamera.z);

    glm::mat4 model(1.0f);
    model = glm::translate(model, particle.position);
    model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(particle.size, particle.size, particle.size));
    const glm::mat4 modelView = view * model;
    glLoadMatrixf(glm::value_ptr(modelView));

    glColor4f(1.0f, 0.85f, 0.2f, alpha);
    glBegin(GL_QUADS);
    glVertex3f(-0.5f, -0.5f, 0.0f);
    glVertex3f(0.5f, -0.5f, 0.0f);
    glVertex3f(0.5f, 0.5f, 0.0f);
    glVertex3f(-0.5f, 0.5f, 0.0f);
    glEnd();
  }

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

bool ParticleSystem::hasEffekseerHook() const {
#if FPS_HAS_EFFEKSEER
  return true;
#else
  return false;
#endif
}

}  // namespace fps::rendering
