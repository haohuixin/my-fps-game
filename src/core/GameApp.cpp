#include "core/GameApp.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "rendering/OpenGLCompat.h"

#include "GL/gl3w.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace fps::core {
namespace {
constexpr float kFixedStep = 1.0f / 120.0f;
}  // namespace

bool GameApp::initialize() {
  const auto finalizeGameplaySetup = [this]() -> bool {
    if (!animationSet_.loadFromXml("assets/enemy_anim.xml")) {
      std::fprintf(stderr, "Failed to load sprite animation XML.\n");
      return false;
    }
    if (!physicsWorld_.initialize()) {
      std::fprintf(stderr, "Failed to initialize physics world.\n");
      return false;
    }
    physicsWorld_.resetPlayer({0.0f, 0.0f, 6.0f});
    camera_.bindToPlayerFeet(physicsWorld_.playerFeetPosition());
    particleSystem_.initialize();

    const auto* idleClip = animationSet_.findClip("idle");
    if (idleClip == nullptr) {
      std::fprintf(stderr, "idle clip missing in enemy_anim.xml\n");
      return false;
    }

    enemies_.clear();
    enemies_.emplace_back(0, glm::vec3(-2.5f, 1.0f, -4.5f), 0.75f, idleClip);
    enemies_.emplace_back(1, glm::vec3(0.0f, 1.0f, -7.0f), 0.75f, idleClip);
    enemies_.emplace_back(2, glm::vec3(3.0f, 1.0f, -5.5f), 0.75f, idleClip);
    for (auto& enemy : enemies_) {
      enemy.update(0.0f, physicsWorld_);
    }
    return true;
  };

  if (!glfwInit()) {
    std::fprintf(stderr, "glfwInit failed, switching to headless gameplay self-check mode.\n");
    headlessMode_ = true;
    return finalizeGameplaySetup();
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
  window_ = glfwCreateWindow(1280, 720, "my-fps-game", nullptr, nullptr);
  if (window_ == nullptr) {
    std::fprintf(stderr, "glfwCreateWindow failed\n");
    return false;
  }

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(1);
  glfwSetWindowUserPointer(window_, this);
  glfwSetKeyCallback(window_, keyCallback);
  glfwSetMouseButtonCallback(window_, mouseButtonCallback);
  glfwSetCursorPosCallback(window_, cursorCallback);
  glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (gl3wInit() != 0) {
    std::fprintf(stderr, "gl3wInit failed\n");
    return false;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  if (!finalizeGameplaySetup()) {
    return false;
  }
  if (!enemyTexture_.loadFromFile("assets/enemy_atlas.png")) {
    std::fprintf(stderr, "Failed to load sprite atlas texture.\n");
    return false;
  }
  audioSystem_.initialize("assets/shoot.wav", "assets/shoot.wav");
  hudLayer_.initialize(window_);
  hudLayer_.setRuntimeFlags(physicsWorld_.hasJoltHook(), particleSystem_.hasEffekseerHook(), false);

  return true;
}

int GameApp::run() {
  if (headlessMode_) {
    for (int step = 0; step < 6; ++step) {
      fixedUpdate(kFixedStep);
    }
    handleShot();
    return score_ > 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }

  float lastTime = static_cast<float>(glfwGetTime());
  float accumulator = 0.0f;

  while (!glfwWindowShouldClose(window_)) {
    const float now = static_cast<float>(glfwGetTime());
    const float frameDt = std::min(now - lastTime, 0.1f);
    lastTime = now;

    glfwPollEvents();
    updateFrame(frameDt);

    accumulator += frameDt;
    while (accumulator >= kFixedStep) {
      fixedUpdate(kFixedStep);
      accumulator -= kFixedStep;
    }

    drawScene();
    glfwSwapBuffers(window_);
  }
  return EXIT_SUCCESS;
}

GameApp::~GameApp() {
  if (window_ != nullptr) {
    glfwDestroyWindow(window_);
  }
  glfwTerminate();
}

void GameApp::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  (void)scancode;
  (void)mods;
  auto* app = static_cast<GameApp*>(glfwGetWindowUserPointer(window));
  if (app != nullptr) {
    app->onKey(key, action);
  }
}

void GameApp::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  (void)mods;
  auto* app = static_cast<GameApp*>(glfwGetWindowUserPointer(window));
  if (app != nullptr) {
    app->onMouseButton(button, action);
  }
}

void GameApp::cursorCallback(GLFWwindow* window, double x, double y) {
  auto* app = static_cast<GameApp*>(glfwGetWindowUserPointer(window));
  if (app != nullptr) {
    app->onCursor(x, y);
  }
}

void GameApp::onKey(int key, int action) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window_, GLFW_TRUE);
    return;
  }
  if (key < 0 || key >= static_cast<int>(input_.keys.size())) {
    return;
  }
  if (action == GLFW_PRESS) {
    input_.keys[static_cast<std::size_t>(key)] = true;
    if (key == GLFW_KEY_SPACE) {
      input_.jumpPressed = true;
    }
  } else if (action == GLFW_RELEASE) {
    input_.keys[static_cast<std::size_t>(key)] = false;
  }
}

void GameApp::onMouseButton(int button, int action) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    input_.shootPressed = true;
  }
}

void GameApp::onCursor(double x, double y) {
  if (!input_.mouseInitialized) {
    input_.lastMouseX = static_cast<float>(x);
    input_.lastMouseY = static_cast<float>(y);
    input_.mouseInitialized = true;
    return;
  }
  input_.mouseDeltaX += static_cast<float>(x) - input_.lastMouseX;
  input_.mouseDeltaY += input_.lastMouseY - static_cast<float>(y);
  input_.lastMouseX = static_cast<float>(x);
  input_.lastMouseY = static_cast<float>(y);
}

void GameApp::updateFrame(float frameDt) {
  camera_.applyMouseDelta(input_.mouseDeltaX, input_.mouseDeltaY);
  input_.mouseDeltaX = 0.0f;
  input_.mouseDeltaY = 0.0f;

  particleSystem_.update(frameDt);
  camera_.bindToPlayerFeet(physicsWorld_.playerFeetPosition());
  hudLayer_.setScore(score_);
}

void GameApp::fixedUpdate(float stepDt) {
  playerController_.captureInput(input_, camera_);
  physicsWorld_.stepCharacter(playerController_.desiredVelocity(), playerController_.consumeJumpPressed(), stepDt);
  camera_.bindToPlayerFeet(physicsWorld_.playerFeetPosition());

  for (auto& enemy : enemies_) {
    enemy.update(stepDt, physicsWorld_);
  }

  if (input_.shootPressed) {
    handleShot();
    input_.shootPressed = false;
  }
  input_.jumpPressed = false;
}

void GameApp::handleShot() {
  audioSystem_.playShoot();
  const auto hit = weaponSystem_.fire(camera_.eyePosition(), camera_.front(), physicsWorld_);
  if (!hit.has_value()) {
    return;
  }

  for (auto& enemy : enemies_) {
    if (enemy.id() != hit->enemyId) {
      continue;
    }
    if (enemy.applyDamage(1)) {
      score_ += 100;
      particleSystem_.triggerHit(hit->position);
      audioSystem_.playHit();
      enemy.update(0.0f, physicsWorld_);
    }
    break;
  }
}

void GameApp::drawScene() const {
  if (headlessMode_) {
    return;
  }

  int framebufferWidth = 1;
  int framebufferHeight = 1;
  glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
  glViewport(0, 0, framebufferWidth, framebufferHeight);

  glClearColor(0.07f, 0.09f, 0.13f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const glm::mat4 projection = glm::perspective(glm::radians(75.0f),
                                                static_cast<float>(framebufferWidth) /
                                                    static_cast<float>(framebufferHeight),
                                                0.1f, 100.0f);
  const glm::mat4 view = camera_.viewMatrix();

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(glm::value_ptr(projection));
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(glm::value_ptr(view));

  drawGround();
  drawSpawnMarkers();
  for (const auto& enemy : enemies_) {
    if (enemy.alive()) {
      drawEnemy(enemy);
    }
  }
  particleSystem_.draw(view, projection, camera_.eyePosition());
  hudLayer_.draw(framebufferWidth, framebufferHeight);
}

void GameApp::drawGround() const {
  glDisable(GL_TEXTURE_2D);
  glColor3f(0.19f, 0.23f, 0.28f);
  glBegin(GL_QUADS);
  glVertex3f(-20.0f, 0.0f, -20.0f);
  glVertex3f(20.0f, 0.0f, -20.0f);
  glVertex3f(20.0f, 0.0f, 20.0f);
  glVertex3f(-20.0f, 0.0f, 20.0f);
  glEnd();

  glColor3f(0.24f, 0.28f, 0.33f);
  glBegin(GL_LINES);
  for (int line = -20; line <= 20; ++line) {
    glVertex3f(static_cast<float>(line), 0.01f, -20.0f);
    glVertex3f(static_cast<float>(line), 0.01f, 20.0f);
    glVertex3f(-20.0f, 0.01f, static_cast<float>(line));
    glVertex3f(20.0f, 0.01f, static_cast<float>(line));
  }
  glEnd();
}

void GameApp::drawSpawnMarkers() const {
  glDisable(GL_TEXTURE_2D);
  glColor3f(0.85f, 0.25f, 0.25f);
  glBegin(GL_LINES);
  for (const auto& enemy : enemies_) {
    const auto& pos = enemy.position();
    glVertex3f(pos.x, 0.0f, pos.z);
    glVertex3f(pos.x, pos.y + 0.8f, pos.z);
  }
  glEnd();
}

void GameApp::drawEnemy(const fps::gameplay::EnemyActor& enemy) const {
  const auto& frame = enemy.currentFrame();
  const glm::vec3& position = enemy.position();
  const glm::vec3 cameraPosition = camera_.eyePosition();
  const glm::vec3 lookDirection = glm::normalize(glm::vec3(cameraPosition.x - position.x, 0.0f,
                                                           cameraPosition.z - position.z));
  const float yaw = std::atan2(lookDirection.x, lookDirection.z);

  glm::mat4 model(1.0f);
  model = glm::translate(model, position);
  model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
  const glm::mat4 modelView = camera_.viewMatrix() * model;

  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(glm::value_ptr(modelView));
  glEnable(GL_TEXTURE_2D);
  enemyTexture_.bind();
  glColor3f(1.0f, 1.0f, 1.0f);
  glBegin(GL_QUADS);
  glTexCoord2f(frame.uvMin.x, frame.uvMin.y);
  glVertex3f(-0.65f, -0.75f, 0.0f);
  glTexCoord2f(frame.uvMax.x, frame.uvMin.y);
  glVertex3f(0.65f, -0.75f, 0.0f);
  glTexCoord2f(frame.uvMax.x, frame.uvMax.y);
  glVertex3f(0.65f, 0.75f, 0.0f);
  glTexCoord2f(frame.uvMin.x, frame.uvMax.y);
  glVertex3f(-0.65f, 0.75f, 0.0f);
  glEnd();
}

}  // namespace fps::core
