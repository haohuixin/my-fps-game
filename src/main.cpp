#include <cstdio>
#include <cstdlib>

#include "core/GameApp.h"

int main() {
  fps::core::GameApp app;
  if (!app.initialize()) {
    std::fprintf(stderr, "Failed to initialize FPS prototype.\n");
    return EXIT_FAILURE;
  }
  return app.run();
}
