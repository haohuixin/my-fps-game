#include "rendering/SpriteAnimation.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <unordered_map>

namespace fps::rendering {
namespace {

using Attributes = std::unordered_map<std::string, std::string>;

Attributes parseAttributes(const std::string& line) {
  static const std::regex kAttributeRegex(R"attr((\w+)="([^"]+)")attr");
  Attributes attributes;
  for (std::sregex_iterator it(line.begin(), line.end(), kAttributeRegex), end; it != end; ++it) {
    attributes[(*it)[1].str()] = (*it)[2].str();
  }
  return attributes;
}

float readFloat(const Attributes& attributes, const std::string& key, float fallback) {
  const auto it = attributes.find(key);
  if (it == attributes.end()) {
    return fallback;
  }
  return std::stof(it->second);
}

int readInt(const Attributes& attributes, const std::string& key, int fallback) {
  const auto it = attributes.find(key);
  if (it == attributes.end()) {
    return fallback;
  }
  return std::stoi(it->second);
}

bool readBool(const Attributes& attributes, const std::string& key, bool fallback) {
  const auto it = attributes.find(key);
  if (it == attributes.end()) {
    return fallback;
  }
  return it->second == "true" || it->second == "1";
}

}  // namespace

bool SpriteAnimationSet::loadFromXml(const std::string& path) {
  clips_.clear();
  texturePath_.clear();

  std::ifstream input(path);
  if (!input.is_open()) {
    return false;
  }

  int atlasWidth = 1;
  int atlasHeight = 1;
  SpriteAnimationClip currentClip;
  bool readingClip = false;
  std::string line;
  while (std::getline(input, line)) {
    if (line.find("<spritesheet") != std::string::npos) {
      const auto attributes = parseAttributes(line);
      auto textureIt = attributes.find("texture");
      if (textureIt != attributes.end()) {
        texturePath_ = textureIt->second;
      }
      atlasWidth = readInt(attributes, "width", 1);
      atlasHeight = readInt(attributes, "height", 1);
    } else if (line.find("<animation") != std::string::npos) {
      const auto attributes = parseAttributes(line);
      currentClip = {};
      currentClip.name = attributes.at("name");
      currentClip.loop = readBool(attributes, "loop", true);
      readingClip = true;
    } else if (line.find("<frame") != std::string::npos && readingClip) {
      const auto attributes = parseAttributes(line);
      const float x = static_cast<float>(readInt(attributes, "x", 0));
      const float y = static_cast<float>(readInt(attributes, "y", 0));
      const float w = static_cast<float>(readInt(attributes, "w", atlasWidth));
      const float h = static_cast<float>(readInt(attributes, "h", atlasHeight));

      SpriteFrame frame;
      frame.uvMin = {x / static_cast<float>(atlasWidth), 1.0f - (y + h) / static_cast<float>(atlasHeight)};
      frame.uvMax = {(x + w) / static_cast<float>(atlasWidth), 1.0f - y / static_cast<float>(atlasHeight)};
      frame.duration = readFloat(attributes, "duration", 0.1f);
      currentClip.frames.push_back(frame);
    } else if (line.find("</animation>") != std::string::npos && readingClip) {
      if (!currentClip.name.empty() && !currentClip.frames.empty()) {
        clips_[currentClip.name] = currentClip;
      }
      readingClip = false;
    }
  }

  return !texturePath_.empty() && !clips_.empty();
}

const SpriteAnimationClip* SpriteAnimationSet::findClip(const std::string& name) const {
  const auto it = clips_.find(name);
  if (it == clips_.end()) {
    return nullptr;
  }
  return &it->second;
}

void SpriteAnimator::play(const SpriteAnimationClip* clip) {
  clip_ = clip;
  frameIndex_ = 0;
  frameTime_ = 0.0f;
}

void SpriteAnimator::update(float dt) {
  if (clip_ == nullptr || clip_->frames.empty()) {
    return;
  }

  frameTime_ += dt;
  while (frameTime_ >= clip_->frames[frameIndex_].duration) {
    frameTime_ -= clip_->frames[frameIndex_].duration;
    if (frameIndex_ + 1 < clip_->frames.size()) {
      ++frameIndex_;
    } else if (clip_->loop) {
      frameIndex_ = 0;
    } else {
      frameTime_ = 0.0f;
      break;
    }
  }
}

const SpriteFrame& SpriteAnimator::currentFrame() const {
  static const SpriteFrame kFallbackFrame{};
  if (clip_ == nullptr || clip_->frames.empty()) {
    return kFallbackFrame;
  }
  return clip_->frames[frameIndex_];
}

}  // namespace fps::rendering
