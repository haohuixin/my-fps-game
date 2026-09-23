#include "bloxorz/Game.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

namespace {
using bloxorz::Game;
using bloxorz::GridPos;
using bloxorz::Level;
using bloxorz::MoveDirection;
using bloxorz::TileType;

constexpr const char* kDefaultTitle = "Bloxorz Demo";
constexpr const char* kWinTitle = "Bloxorz Demo - Cleared! Press R to restart";

struct Vertex {
  float position[3];
  float normal[3];
};

struct VSParams {
  HMM_Mat4 mvp;
  HMM_Mat4 model;
};

struct FSParams {
  HMM_Vec4 color;
  HMM_Vec4 lightDir;
};

struct AppState {
  Game game;
  sg_pipeline pipeline{};
  sg_bindings bindings{};
  sg_pass_action passAction{};
  HMM_Mat4 view{};
  HMM_Mat4 projection{};
  bool titleShowsWin = false;
};

AppState gApp;

constexpr std::array<Vertex, 24> kCubeVertices = {{
    {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
    {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
    {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
}};

constexpr std::array<std::uint16_t, 36> kCubeIndices = {{
    0, 1, 2, 0, 2, 3,
    4, 5, 6, 4, 6, 7,
    8, 9, 10, 8, 10, 11,
    12, 13, 14, 12, 14, 15,
    16, 17, 18, 16, 18, 19,
    20, 21, 22, 20, 22, 23,
}};

sg_shader makeShader() {
  sg_shader_desc desc{};
  desc.vertex_func.source =
      "#version 330\n"
      "layout(std140) uniform vs_params {\n"
      "  mat4 mvp;\n"
      "  mat4 model;\n"
      "};\n"
      "layout(location=0) in vec3 position;\n"
      "layout(location=1) in vec3 normal;\n"
      "out vec3 v_normal;\n"
      "void main() {\n"
      "  gl_Position = mvp * vec4(position, 1.0);\n"
      "  v_normal = mat3(model) * normal;\n"
      "}\n";
  desc.fragment_func.source =
      "#version 330\n"
      "layout(std140) uniform fs_params {\n"
      "  vec4 color;\n"
      "  vec4 light_dir;\n"
      "};\n"
      "in vec3 v_normal;\n"
      "out vec4 frag_color;\n"
      "void main() {\n"
      "  float diffuse = max(dot(normalize(v_normal), normalize(light_dir.xyz)), 0.0);\n"
      "  float lighting = 0.25 + diffuse * 0.75;\n"
      "  frag_color = vec4(color.rgb * lighting, color.a);\n"
      "}\n";
  desc.attrs[0].base_type = SG_SHADERATTRBASETYPE_FLOAT;
  desc.attrs[0].glsl_name = "position";
  desc.attrs[1].base_type = SG_SHADERATTRBASETYPE_FLOAT;
  desc.attrs[1].glsl_name = "normal";
  desc.uniform_blocks[0].stage = SG_SHADERSTAGE_VERTEX;
  desc.uniform_blocks[0].size = sizeof(VSParams);
  desc.uniform_blocks[0].layout = SG_UNIFORMLAYOUT_NATIVE;
  desc.uniform_blocks[0].glsl_uniforms[0].type = SG_UNIFORMTYPE_MAT4;
  desc.uniform_blocks[0].glsl_uniforms[0].glsl_name = "mvp";
  desc.uniform_blocks[0].glsl_uniforms[1].type = SG_UNIFORMTYPE_MAT4;
  desc.uniform_blocks[0].glsl_uniforms[1].glsl_name = "model";
  desc.uniform_blocks[1].stage = SG_SHADERSTAGE_FRAGMENT;
  desc.uniform_blocks[1].size = sizeof(FSParams);
  desc.uniform_blocks[1].layout = SG_UNIFORMLAYOUT_NATIVE;
  desc.uniform_blocks[1].glsl_uniforms[0].type = SG_UNIFORMTYPE_FLOAT4;
  desc.uniform_blocks[1].glsl_uniforms[0].glsl_name = "color";
  desc.uniform_blocks[1].glsl_uniforms[1].type = SG_UNIFORMTYPE_FLOAT4;
  desc.uniform_blocks[1].glsl_uniforms[1].glsl_name = "light_dir";
  desc.label = "bloxorz-shader";
  return sg_make_shader(&desc);
}

void updateWindowTitle() {
  const bool won = gApp.game.won();
  if (won == gApp.titleShowsWin) {
    return;
  }
  gApp.titleShowsWin = won;
  sapp_set_window_title(won ? kWinTitle : kDefaultTitle);
}

void drawMesh(const HMM_Mat4& model, const HMM_Vec4& color) {
  const HMM_Mat4 vp = HMM_MulM4(gApp.projection, gApp.view);
  const VSParams vsParams{.mvp = HMM_MulM4(vp, model), .model = model};
  const FSParams fsParams{.color = color, .lightDir = HMM_V4(0.4f, 1.0f, 0.3f, 0.0f)};

  sg_apply_uniforms(0, SG_RANGE_REF(vsParams));
  sg_apply_uniforms(1, SG_RANGE_REF(fsParams));
  sg_draw(0, static_cast<int>(kCubeIndices.size()), 1);
}

void init(void) {
  sg_desc desc{};
  desc.environment = sglue_environment();
  desc.logger.func = slog_func;
  sg_setup(&desc);

  if (!sg_isvalid()) {
    std::fprintf(stderr, "sokol_gfx initialization failed\n");
    return;
  }

  sg_buffer_desc vbDesc{};
  vbDesc.data = SG_RANGE(kCubeVertices);
  vbDesc.label = "cube-vertices";
  gApp.bindings.vertex_buffers[0] = sg_make_buffer(&vbDesc);

  sg_buffer_desc ibDesc{};
  ibDesc.usage.index_buffer = true;
  ibDesc.data = SG_RANGE(kCubeIndices);
  ibDesc.label = "cube-indices";
  gApp.bindings.index_buffer = sg_make_buffer(&ibDesc);

  const sg_shader shader = makeShader();
  sg_pipeline_desc pipDesc{};
  pipDesc.shader = shader;
  pipDesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
  pipDesc.layout.attrs[0].offset = offsetof(Vertex, position);
  pipDesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
  pipDesc.layout.attrs[1].offset = offsetof(Vertex, normal);
  pipDesc.index_type = SG_INDEXTYPE_UINT16;
  pipDesc.cull_mode = SG_CULLMODE_BACK;
  pipDesc.face_winding = SG_FACEWINDING_CCW;
  pipDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
  pipDesc.depth.write_enabled = true;
  pipDesc.label = "bloxorz-pipeline";
  gApp.pipeline = sg_make_pipeline(&pipDesc);

  gApp.passAction.colors[0].load_action = SG_LOADACTION_CLEAR;
  gApp.passAction.colors[0].clear_value = {0.10f, 0.12f, 0.18f, 1.0f};
  gApp.passAction.depth.load_action = SG_LOADACTION_CLEAR;
  gApp.passAction.depth.clear_value = 1.0f;

  updateWindowTitle();
}

void frame(void) {
  const float dt = static_cast<float>(sapp_frame_duration());
  gApp.game.update(dt);
  updateWindowTitle();

  const float width = sapp_widthf();
  const float height = sapp_heightf();
  if (width <= 0.0f || height <= 0.0f) {
    return;
  }
  const float aspect = width / height;
  gApp.projection = HMM_Perspective_RH_NO(0.9f, aspect, 0.1f, 100.0f);

  const Level& level = gApp.game.level();
  const HMM_Vec3 boardCenter = HMM_V3(static_cast<float>(level.width - 1) * 0.5f, 0.0f,
                                      static_cast<float>(level.height - 1) * 0.5f);
  const HMM_Vec3 eye = HMM_AddV3(boardCenter, HMM_V3(4.5f, 7.0f, 7.0f));
  gApp.view = HMM_LookAt_RH(eye, HMM_AddV3(boardCenter, HMM_V3(0.0f, 0.75f, 0.0f)), HMM_V3(0.0f, 1.0f, 0.0f));

  sg_pass pass{};
  pass.action = gApp.passAction;
  pass.swapchain = sglue_swapchain();
  sg_begin_pass(&pass);
  sg_apply_pipeline(gApp.pipeline);
  sg_apply_bindings(&gApp.bindings);

  for (int z = 0; z < level.height; ++z) {
    for (int x = 0; x < level.width; ++x) {
      const TileType tile = level.tileAt({x, z});
      if (tile == TileType::Void) {
        continue;
      }
      const HMM_Vec4 color = tile == TileType::Goal ? HMM_V4(0.92f, 0.75f, 0.22f, 1.0f)
                                                    : HMM_V4(0.34f, 0.44f, 0.58f, 1.0f);
      drawMesh(Game::tileModelMatrix({x, z}, tile), color);
    }
  }

  const HMM_Vec4 blockColor = gApp.game.won() ? HMM_V4(0.34f, 0.86f, 0.48f, 1.0f)
                                              : HMM_V4(0.74f, 0.28f, 0.24f, 1.0f);
  drawMesh(gApp.game.blockModelMatrix(), blockColor);

  sg_end_pass();
  sg_commit();
}

void cleanup(void) {
  sg_shutdown();
}

void handleMove(MoveDirection direction) {
  (void)gApp.game.queueMove(direction);
}

void event(const sapp_event* event) {
  if (event->type != SAPP_EVENTTYPE_KEY_DOWN || event->key_repeat) {
    return;
  }

  switch (event->key_code) {
    case SAPP_KEYCODE_ESCAPE:
      sapp_request_quit();
      break;
    case SAPP_KEYCODE_LEFT:
    case SAPP_KEYCODE_A:
      handleMove(MoveDirection::Left);
      break;
    case SAPP_KEYCODE_RIGHT:
    case SAPP_KEYCODE_D:
      handleMove(MoveDirection::Right);
      break;
    case SAPP_KEYCODE_UP:
    case SAPP_KEYCODE_W:
      handleMove(MoveDirection::Up);
      break;
    case SAPP_KEYCODE_DOWN:
    case SAPP_KEYCODE_S:
      handleMove(MoveDirection::Down);
      break;
    case SAPP_KEYCODE_R:
      gApp.game.reset();
      updateWindowTitle();
      break;
    default:
      break;
  }
}

}  // namespace

sapp_desc sokol_main(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  sapp_desc desc{};
  desc.init_cb = init;
  desc.frame_cb = frame;
  desc.cleanup_cb = cleanup;
  desc.event_cb = event;
  desc.width = 960;
  desc.height = 720;
  desc.sample_count = 4;
  desc.window_title = kDefaultTitle;
  desc.logger.func = slog_func;
  return desc;
}
