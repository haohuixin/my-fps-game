#include "rendering/Texture2D.h"

#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace fps::rendering {

Texture2D::Texture2D(Texture2D&& other) noexcept {
  *this = std::move(other);
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  release();
  textureId_ = std::exchange(other.textureId_, 0);
  width_ = std::exchange(other.width_, 0);
  height_ = std::exchange(other.height_, 0);
  return *this;
}

Texture2D::~Texture2D() {
  release();
}

bool Texture2D::loadFromFile(const std::string& path) {
  release();
  int components = 0;
  stbi_uc* pixels = stbi_load(path.c_str(), &width_, &height_, &components, 4);
  if (pixels == nullptr) {
    width_ = 0;
    height_ = 0;
    return false;
  }

  glGenTextures(1, &textureId_);
  glBindTexture(GL_TEXTURE_2D, textureId_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  stbi_image_free(pixels);
  return true;
}

void Texture2D::bind() const {
  glBindTexture(GL_TEXTURE_2D, textureId_);
}

void Texture2D::release() {
  if (textureId_ != 0) {
    glDeleteTextures(1, &textureId_);
    textureId_ = 0;
  }
}

}  // namespace fps::rendering
