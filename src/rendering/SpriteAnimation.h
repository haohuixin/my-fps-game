#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/vec2.hpp>

namespace fps::rendering {

struct SpriteFrame {
  glm::vec2 uvMin{0.0f, 0.0f};
  glm::vec2 uvMax{1.0f, 1.0f};
  float duration = 0.1f;
};

struct SpriteAnimationClip {
  std::string name;
  bool loop = true;
  std::vector<SpriteFrame> frames;
};

class SpriteAnimationSet {
 public:
  bool loadFromXml(const std::string& path);
  [[nodiscard]] const SpriteAnimationClip* findClip(const std::string& name) const;
  [[nodiscard]] const std::string& texturePath() const { return texturePath_; }

 private:
  std::string texturePath_;
  std::unordered_map<std::string, SpriteAnimationClip> clips_;
};

class SpriteAnimator {
 public:
  void play(const SpriteAnimationClip* clip);
  void update(float dt);
  [[nodiscard]] const SpriteFrame& currentFrame() const;

 private:
  const SpriteAnimationClip* clip_ = nullptr;
  std::size_t frameIndex_ = 0;
  float frameTime_ = 0.0f;
};

}  // namespace fps::rendering
