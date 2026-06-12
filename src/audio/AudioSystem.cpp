#include "audio/AudioSystem.h"

#include <new>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace fps::audio {

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem() {
  if (shootSound_ != nullptr) {
    ma_sound_uninit(shootSound_);
    delete shootSound_;
  }
  if (hitSound_ != nullptr) {
    ma_sound_uninit(hitSound_);
    delete hitSound_;
  }
  if (engine_ != nullptr) {
    ma_engine_uninit(engine_);
    delete engine_;
  }
}

bool AudioSystem::initialize(const std::string& shootPath, const std::string& hitPath) {
  engine_ = new (std::nothrow) ma_engine();
  shootSound_ = new (std::nothrow) ma_sound();
  hitSound_ = new (std::nothrow) ma_sound();
  if (engine_ == nullptr || shootSound_ == nullptr || hitSound_ == nullptr) {
    return false;
  }

  if (ma_engine_init(nullptr, engine_) != MA_SUCCESS) {
    return false;
  }

  const bool shootLoaded = ma_sound_init_from_file(engine_, shootPath.c_str(), MA_SOUND_FLAG_DECODE, nullptr,
                                                   nullptr, shootSound_) == MA_SUCCESS;
  hitEnabled_ = ma_sound_init_from_file(engine_, hitPath.c_str(), MA_SOUND_FLAG_DECODE, nullptr, nullptr,
                                        hitSound_) == MA_SUCCESS;
  enabled_ = shootLoaded;
  return enabled_;
}

void AudioSystem::playShoot() {
  if (!enabled_ || shootSound_ == nullptr) {
    return;
  }
  ma_sound_seek_to_pcm_frame(shootSound_, 0);
  ma_sound_start(shootSound_);
}

void AudioSystem::playHit() {
  if (!hitEnabled_ || hitSound_ == nullptr) {
    return;
  }
  ma_sound_seek_to_pcm_frame(hitSound_, 0);
  ma_sound_start(hitSound_);
}

}  // namespace fps::audio
