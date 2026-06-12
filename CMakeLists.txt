cmake_minimum_required(VERSION 3.20)
project(my_fps_game LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

find_package(OpenGL REQUIRED)
find_package(Jolt CONFIG QUIET)
find_package(Effekseer QUIET)
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
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw glm)

add_library(gl3w STATIC third_party/gl3w/src/gl3w.c)
target_include_directories(gl3w PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/gl3w/include)

add_executable(fps_game src/main.cpp)
target_include_directories(fps_game PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/miniaudio
)
target_link_libraries(fps_game PRIVATE glfw OpenGL::GL glm::glm gl3w)

if(TARGET Jolt)
  target_link_libraries(fps_game PRIVATE Jolt)
  target_compile_definitions(fps_game PRIVATE FPS_HAS_JOLT=1)
endif()
if(TARGET Effekseer)
  target_link_libraries(fps_game PRIVATE Effekseer)
  target_compile_definitions(fps_game PRIVATE FPS_HAS_EFFEKSEER=1)
endif()
if(TARGET nanogui)
  target_link_libraries(fps_game PRIVATE nanogui)
  target_compile_definitions(fps_game PRIVATE FPS_HAS_NANOGUI=1)
endif()
