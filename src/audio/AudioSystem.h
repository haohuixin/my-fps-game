#pragma once

#include <string>

struct ma_engine;
struct ma_sound;

namespace fps::audio {

class AudioSystem {
 public:
  AudioSystem();
  AudioSystem(const AudioSystem&) = delete;
  AudioSystem& operator=(const AudioSystem&) = delete;
  ~AudioSystem();

  bool initialize(const std::string& shootPath, const std::string& hitPath);
  void playShoot();
  void playHit();
  [[nodiscard]] bool enabled() const { return enabled_; }

 private:
  ma_engine* engine_ = nullptr;
  ma_sound* shootSound_ = nullptr;
  ma_sound* hitSound_ = nullptr;
  bool enabled_ = false;
  bool hitEnabled_ = false;
};

}  // namespace fps::audio
