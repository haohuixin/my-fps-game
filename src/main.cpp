#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "GL/gl3w.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#if FPS_HAS_NANOGUI
#include <nanogui/nanogui.h>
#endif
#if FPS_HAS_EFFEKSEER
#include <Effekseer.h>
#endif
#if FPS_HAS_JOLT
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#endif

struct InputState {
  bool keys[1024]{};
  bool jumpPressed = false;
  bool shootPressed = false;
  float mouseDeltaX = 0.0f;
  float mouseDeltaY = 0.0f;
};

struct Enemy {
  glm::vec3 pos{};
  float radius = 0.5f;
  bool alive = true;
};

struct PlayerState {
  glm::vec3 pos{0.0f, 1.0f, 6.0f};
  glm::vec3 vel{};
  float yaw = -90.0f;
  float pitch = 0.0f;
};

class Game {
 public:
  bool init() {
    if (!glfwInit()) {
      std::fprintf(stderr, "glfwInit failed\n");
      return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    window_ = glfwCreateWindow(1280, 720, "my-fps-game", nullptr, nullptr);
    if (!window_) {
      std::fprintf(stderr, "glfwCreateWindow failed\n");
      return false;
    }
    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, keyCallback);
    glfwSetCursorPosCallback(window_, cursorCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (gl3wInit() != 0) {
      std::fprintf(stderr, "gl3wInit failed\n");
      return false;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    if (!loadEnemyTexture("assets/enemy.png")) {
      std::fprintf(stderr, "enemy texture load failed\n");
      return false;
    }
    initAudio("assets/shoot.wav");

    enemies_.push_back(Enemy{glm::vec3(0.0f, 1.0f, -2.0f), 0.7f, true});
#if FPS_HAS_JOLT
    // Jolt 参与场景时，敌人/玩家应由刚体或 Character 同步（此处保持最小接入点）。
    joltEnabled_ = true;
#endif
    return true;
  }

  int run() {
    float last = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window_)) {
      float now = (float)glfwGetTime();
      float dt = now - last;
      last = now;

      glfwPollEvents();
      update(dt);
      draw();
      glfwSwapBuffers(window_);
    }
    return 0;
  }

  ~Game() {
    if (audioEnabled_) ma_sound_uninit(&shootSound_);
    ma_engine_uninit(&audioEngine_);
    if (enemyTexture_ != 0) glDeleteTextures(1, &enemyTexture_);
    if (window_) glfwDestroyWindow(window_);
    glfwTerminate();
  }

 private:
  static void keyCallback(GLFWwindow* win, int key, int, int action, int) {
    auto* self = reinterpret_cast<Game*>(glfwGetWindowUserPointer(win));
    if (!self || key < 0 || key >= 1024) return;
    if (action == GLFW_PRESS) {
      self->input_.keys[key] = true;
      if (key == GLFW_KEY_SPACE) self->input_.jumpPressed = true;
    } else if (action == GLFW_RELEASE) {
      self->input_.keys[key] = false;
    }
  }

  static void mouseButtonCallback(GLFWwindow* win, int button, int action, int) {
    auto* self = reinterpret_cast<Game*>(glfwGetWindowUserPointer(win));
    if (!self) return;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      self->input_.shootPressed = true;
    }
  }

  static void cursorCallback(GLFWwindow* win, double x, double y) {
    auto* self = reinterpret_cast<Game*>(glfwGetWindowUserPointer(win));
    if (!self) return;
    if (!self->mouseInited_) {
      self->lastMouseX_ = (float)x;
      self->lastMouseY_ = (float)y;
      self->mouseInited_ = true;
      return;
    }
    self->input_.mouseDeltaX += (float)(x - self->lastMouseX_);
    self->input_.mouseDeltaY += (float)(self->lastMouseY_ - y);
    self->lastMouseX_ = (float)x;
    self->lastMouseY_ = (float)y;
  }

  bool loadEnemyTexture(const char* path) {
    int w = 0, h = 0, c = 0;
    stbi_uc* data = stbi_load(path, &w, &h, &c, 4);
    if (!data) return false;
    glGenTextures(1, &enemyTexture_);
    glBindTexture(GL_TEXTURE_2D, enemyTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return true;
  }

  bool initAudio(const char* shootWav) {
    if (ma_engine_init(nullptr, &audioEngine_) != MA_SUCCESS) return false;
    audioEnabled_ = ma_sound_init_from_file(&audioEngine_, shootWav, MA_SOUND_FLAG_DECODE, nullptr,
                                            nullptr, &shootSound_) == MA_SUCCESS;
    return true;
  }

  glm::vec3 cameraFront() const {
    glm::vec3 front;
    front.x = std::cos(glm::radians(player_.yaw)) * std::cos(glm::radians(player_.pitch));
    front.y = std::sin(glm::radians(player_.pitch));
    front.z = std::sin(glm::radians(player_.yaw)) * std::cos(glm::radians(player_.pitch));
    return glm::normalize(front);
  }

  void update(float dt) {
    constexpr float kMouseSense = 0.1f;
    player_.yaw += input_.mouseDeltaX * kMouseSense;
    player_.pitch += input_.mouseDeltaY * kMouseSense;
    player_.pitch = std::clamp(player_.pitch, -89.0f, 89.0f);
    input_.mouseDeltaX = input_.mouseDeltaY = 0.0f;

    glm::vec3 forward = cameraFront();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.01f) forward = glm::normalize(forward);
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    glm::vec3 wish{};
    if (input_.keys[GLFW_KEY_W]) wish += forward;
    if (input_.keys[GLFW_KEY_S]) wish -= forward;
    if (input_.keys[GLFW_KEY_A]) wish -= right;
    if (input_.keys[GLFW_KEY_D]) wish += right;
    if (glm::length(wish) > 0.01f) wish = glm::normalize(wish);

    player_.vel.x = wish.x * 5.0f;
    player_.vel.z = wish.z * 5.0f;
    player_.vel.y += -15.0f * dt;
    if (player_.pos.y <= 1.0f) {
      player_.pos.y = 1.0f;
      player_.vel.y = std::max(0.0f, player_.vel.y);
      if (input_.jumpPressed) player_.vel.y = 6.0f;
    }
    input_.jumpPressed = false;
    player_.pos += player_.vel * dt;

    if (input_.shootPressed) {
      if (audioEnabled_) {
        ma_sound_seek_to_pcm_frame(&shootSound_, 0);
        ma_sound_start(&shootSound_);
      }
      tryShoot();
    }
    input_.shootPressed = false;

    std::string title = "my-fps-game | score=" + std::to_string(score_);
    glfwSetWindowTitle(window_, title.c_str());
  }

  void tryShoot() {
    glm::vec3 origin = player_.pos;
    glm::vec3 dir = cameraFront();
    for (auto& e : enemies_) {
      if (!e.alive) continue;
      glm::vec3 oc = origin - e.pos;
      float a = glm::dot(dir, dir);
      float b = 2.0f * glm::dot(oc, dir);
      float c = glm::dot(oc, oc) - e.radius * e.radius;
      float disc = b * b - 4.0f * a * c;
      if (disc >= 0.0f) {
        e.alive = false;
        score_ += 10;
        hitFlash_ = 0.15f;
#if FPS_HAS_EFFEKSEER
        // Effekseer 命中效果接入点：可在这里发射粒子到 e.pos。
#endif
        break;
      }
    }
  }

  void drawQuadBillboard(const Enemy& e, const glm::mat4& view, const glm::mat4& proj) {
    glm::vec3 look = glm::normalize(glm::vec3(player_.pos.x - e.pos.x, 0.0f, player_.pos.z - e.pos.z));
    float yaw = std::atan2(look.x, look.z);
    glm::mat4 model(1.0f);
    model = glm::translate(model, e.pos);
    model = glm::rotate(model, yaw, glm::vec3(0, 1, 0));

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&proj[0][0]);
    glMatrixMode(GL_MODELVIEW);
    glm::mat4 mv = view * model;
    glLoadMatrixf(&mv[0][0]);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, enemyTexture_);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-0.6f, -0.6f, 0.0f);
    glTexCoord2f(1, 0); glVertex3f( 0.6f, -0.6f, 0.0f);
    glTexCoord2f(1, 1); glVertex3f( 0.6f,  0.6f, 0.0f);
    glTexCoord2f(0, 1); glVertex3f(-0.6f,  0.6f, 0.0f);
    glEnd();
  }

  void draw() {
    hitFlash_ = std::max(0.0f, hitFlash_ - 0.016f);
    glClearColor(0.1f + hitFlash_, 0.12f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int w = 1, h = 1;
    glfwGetFramebufferSize(window_, &w, &h);
    glViewport(0, 0, w, h);
    glm::mat4 proj = glm::perspective(glm::radians(75.0f), (float)w / (float)h, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(player_.pos, player_.pos + cameraFront(), glm::vec3(0, 1, 0));

    // simple ground
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&proj[0][0]);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&view[0][0]);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.25f, 0.25f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-20, 0, -20); glVertex3f(20, 0, -20);
    glVertex3f(20, 0, 20); glVertex3f(-20, 0, 20);
    glEnd();

    for (const auto& e : enemies_) if (e.alive) drawQuadBillboard(e, view, proj);
  }

  GLFWwindow* window_ = nullptr;
  InputState input_{};
  PlayerState player_{};
  std::vector<Enemy> enemies_{};
  int score_ = 0;
  float hitFlash_ = 0.0f;
  bool mouseInited_ = false;
  float lastMouseX_ = 0.0f;
  float lastMouseY_ = 0.0f;

  GLuint enemyTexture_ = 0;
  ma_engine audioEngine_{};
  ma_sound shootSound_{};
  bool audioEnabled_ = false;
  bool joltEnabled_ = false;
};

int main() {
  Game game;
  if (!game.init()) {
    std::fprintf(stderr, "Failed to initialize game.\n");
    return EXIT_FAILURE;
  }
  return game.run();
}
