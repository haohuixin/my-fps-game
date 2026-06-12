#pragma once

#include <string>

#include <GL/gl.h>

namespace fps::rendering {

class Texture2D {
 public:
  Texture2D() = default;
  Texture2D(const Texture2D&) = delete;
  Texture2D& operator=(const Texture2D&) = delete;
  Texture2D(Texture2D&& other) noexcept;
  Texture2D& operator=(Texture2D&& other) noexcept;
  ~Texture2D();

  bool loadFromFile(const std::string& path);
  void bind() const;
  [[nodiscard]] int width() const { return width_; }
  [[nodiscard]] int height() const { return height_; }
  [[nodiscard]] bool valid() const { return textureId_ != 0; }

 private:
  void release();

  unsigned int textureId_ = 0;
  int width_ = 0;
  int height_ = 0;
};

}  // namespace fps::rendering
