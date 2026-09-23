cmake_minimum_required(VERSION 3.20)
project(my_fps_game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
include(CheckIncludeFileCXX)

find_library(FPS_OPENGL_LIB NAMES GL libGL.so.1 REQUIRED)
find_package(X11 QUIET)
find_package(Threads REQUIRED)
find_package(Jolt CONFIG QUIET)
find_package(Effekseer CONFIG QUIET)
find_package(nanogui CONFIG QUIET)

FetchContent_Declare(glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ${X11_FOUND} CACHE BOOL "" FORCE)
if(NOT X11_FOUND)
  message(WARNING "X11 development files not found; GLFW will build with the null platform backend for headless environments.")
endif()
FetchContent_MakeAvailable(glfw glm)

add_library(gl3w STATIC third_party/gl3w/src/gl3w.c)
target_include_directories(gl3w PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gl3w/include)

set(FPS_HAS_JOLT 0)
set(FPS_HAS_EFFEKSEER 0)
set(FPS_HAS_NANOGUI 0)

if(TARGET Jolt)
  set(FPS_HAS_JOLT 1)
endif()
if(TARGET Effekseer)
  set(FPS_HAS_EFFEKSEER 1)
endif()
if(TARGET nanogui)
  set(FPS_HAS_NANOGUI 1)
endif()

set(FPS_SOURCES
  src/main.cpp
  src/core/GameApp.cpp
  src/rendering/Texture2D.cpp
  src/rendering/SpriteAnimation.cpp
  src/rendering/ParticleSystem.cpp
  src/gameplay/FirstPersonCamera.cpp
  src/gameplay/PlayerController.cpp
  src/gameplay/EnemyActor.cpp
  src/gameplay/WeaponSystem.cpp
  src/physics/PhysicsWorld.cpp
  src/audio/AudioSystem.cpp
  src/ui/HudLayer.cpp
)

add_executable(fps_game ${FPS_SOURCES})
target_include_directories(fps_game PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio
)
target_compile_definitions(fps_game PRIVATE
  FPS_HAS_JOLT=${FPS_HAS_JOLT}
  FPS_HAS_EFFEKSEER=${FPS_HAS_EFFEKSEER}
  FPS_HAS_NANOGUI=${FPS_HAS_NANOGUI}
)
target_link_libraries(fps_game PRIVATE glfw glm::glm gl3w ${FPS_OPENGL_LIB})

if(TARGET Jolt)
  target_link_libraries(fps_game PRIVATE Jolt)
endif()
if(TARGET Effekseer)
  target_link_libraries(fps_game PRIVATE Effekseer)
endif()
if(TARGET nanogui)
  target_link_libraries(fps_game PRIVATE nanogui)
endif()

set(BLOXORZ_LOGIC_SOURCES
  src/bloxorz/Game.cpp
)

enable_testing()

add_executable(bloxorz_logic_tests
  tests/bloxorz_logic_tests.cpp
  ${BLOXORZ_LOGIC_SOURCES}
)
target_include_directories(bloxorz_logic_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/handmade_math
)
add_test(NAME bloxorz_logic_tests COMMAND bloxorz_logic_tests)

option(BLOXORZ_BUILD_APP "Build the Sokol Bloxorz demo app when native desktop headers are available" ON)
check_include_file_cxx("GL/gl.h" BLOXORZ_HAS_OPENGL_HEADERS)
check_include_file_cxx("X11/Xlib.h" BLOXORZ_HAS_X11_HEADERS)

if(BLOXORZ_BUILD_APP)
  if(UNIX AND NOT APPLE AND X11_FOUND AND BLOXORZ_HAS_OPENGL_HEADERS AND BLOXORZ_HAS_X11_HEADERS)
    set(BLOXORZ_X11_LIBS ${X11_LIBRARIES})
    if(DEFINED X11_X11_LIB)
      list(APPEND BLOXORZ_X11_LIBS ${X11_X11_LIB})
    endif()
    if(DEFINED X11_Xi_LIB)
      list(APPEND BLOXORZ_X11_LIBS ${X11_Xi_LIB})
    endif()
    if(DEFINED X11_Xcursor_LIB)
      list(APPEND BLOXORZ_X11_LIBS ${X11_Xcursor_LIB})
    endif()
    list(REMOVE_DUPLICATES BLOXORZ_X11_LIBS)

    add_executable(bloxorz_game
      src/bloxorz/App.cpp
      src/bloxorz/SokolImpl.cpp
      ${BLOXORZ_LOGIC_SOURCES}
    )
    target_include_directories(bloxorz_game PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
      ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sokol
      ${CMAKE_CURRENT_SOURCE_DIR}/third_party/handmade_math
    )
    target_link_libraries(bloxorz_game PRIVATE
      ${FPS_OPENGL_LIB}
      ${BLOXORZ_X11_LIBS}
      ${CMAKE_DL_LIBS}
      Threads::Threads
      m
    )
  else()
    message(WARNING
      "Skipping bloxorz_game target: install Linux desktop OpenGL/X11 development headers "
      "(for example libgl1-mesa-dev and xorg-dev) to build the Sokol windowed demo.")
  endif()
endif()
