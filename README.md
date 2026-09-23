# my-fps-game

一个可运行的 C++ FPS 原型，并新增了一个独立的 **Bloxorz 3D 解谜演示**。仓库目前包含两条相互独立的入口：

- `fps_game`：原有 GLFW / OpenGL / GLM FPS 原型
- `bloxorz_game`：新增 Sokol / HandmadeMath Bloxorz 演示（本次仓库内已接成 Linux 桌面目标，在具备 Linux OpenGL/X11 开发头时启用）

FPS 原型继续按 `core / rendering / physics / gameplay / audio / ui` 模块拆分，并使用/预留以下技术栈接入点：

- OpenGL 3.3（兼容模式上下文，便于原型阶段快速渲染）
- GLFW3
- GLM
- gl3w
- stb_image
- miniaudio
- Sokol (`sokol_app.h` / `sokol_gfx.h` / `sokol_glue.h`)
- HandmadeMath
- Jolt Physics（可选接入点）
- Effekseer（可选接入点）
- nanogui（可选接入点）

当前 Bloxorz 的 CMake 接线明确面向 **Linux 桌面**（与现有 README 的构建说明范围一致）；代码本身保持 Sokol 风格的独立入口，但本次 PR 没有额外扩展 Windows/macOS 的仓库级构建脚本。

## 当前 FPS 原型内容

- 第一人称相机绑定到玩家角色脚底位置，并保持 `视角更新` 与 `固定物理步进` 分离
- `W/A/S/D` 平面移动、`Space` 跳跃、鼠标控制 yaw / pitch（pitch 已限制）
- 最小 3D 测试场景：地面、多个敌人生成点/初始敌人
- 屏幕中心射线射击，命中敌人后：
  - 敌人死亡
  - 分数 +100
  - 触发内置命中粒子效果（Effekseer 接入点已封装）
  - 播放命中音效（当前使用占位资源）
- 敌人以 **sprite sheet + XML** 驱动的 2D 像素动画显示在 3D 空间 billboard quad 上
- HUD：屏幕内准星与数字得分显示（并保留 NanoGUI 接入层说明）

## 新增 Bloxorz 演示内容

- 小型默认关卡，格子支持：普通地板、空格/边界、目标格
- 方块三种离散姿态：
  - 竖直：占 1 格，高度 2 格
  - 沿 X 轴躺倒：占 2 格
  - 沿 Z 轴躺倒：占 2 格
- `W/A/S/D` 或方向键控制移动，移动动画期间忽略新输入
- `R` 重置关卡，完全站上目标格后进入胜利状态
- `src/bloxorz/Game.*` 中离散逻辑与动画状态分离：
  - 离散逻辑：方块姿态、占用格、合法/非法移动、胜利判定
  - 动画状态：起止状态、支点、旋转轴、旋转角度、持续时间
- 动画不是瞬移，也不是绕中心旋转；渲染时使用：

```text
model = translate(pivot) * rotate(angle) * translate(-pivot) * start_block_model
```

其中 `pivot` 是当前运动方向对应的底部实际支点边缘，角度随 `delta time` 用 smoothstep 缓动推进。

## 目录结构

```text
src/
  core/        应用主循环、窗口、固定步进
  rendering/   纹理、精灵动画、粒子
  physics/     玩家/敌人碰撞与射线查询封装（Jolt 接入点）
  gameplay/    相机、玩家控制、武器、敌人
  audio/       miniaudio 封装
  ui/          HUD 层
  bloxorz/     Bloxorz 逻辑、Sokol 入口、渲染
tests/
  bloxorz_logic_tests.cpp
assets/
  enemy_atlas.png
  enemy_anim.xml
  shoot.wav
third_party/
  gl3w/
  stb/
  miniaudio/
  sokol/
  handmade_math/
```

## 构建

Linux 下建议先安装基础 OpenGL/X11 开发包（`fps_game` 与 `bloxorz_game` 都建议安装）：

```bash
sudo apt-get update
sudo apt-get install -y libgl1-mesa-dev xorg-dev
```

然后在仓库根目录执行：

```bash
cmake -S . -B build
cmake --build build -j
```

### 仅构建/验证 Bloxorz 逻辑自检

无图形环境下也可以直接验证新增逻辑：

```bash
cmake -S . -B build
cmake --build build --target bloxorz_logic_tests -j
ctest --test-dir build --output-on-failure -R bloxorz_logic_tests
```

### 构建 Bloxorz 窗口演示

当系统已安装 `libgl1-mesa-dev` 与 `xorg-dev` 这类开发包时，CMake 会自动生成 `bloxorz_game`：

```bash
cmake -S . -B build
cmake --build build --target bloxorz_game -j
```

如果当前环境缺少 `GL/gl.h` 或 `X11/Xlib.h`，CMake 会保留 `bloxorz_logic_tests`，并跳过 `bloxorz_game`，避免破坏原有工程构建路径。

## 运行

### FPS 原型

```bash
./build/fps_game
```

### Bloxorz 演示

如果 `bloxorz_game` 已成功生成：

```bash
./build/bloxorz_game
```

## 操作

### FPS 原型

- `W/A/S/D`：移动
- `Space`：跳跃
- 鼠标移动：第一人称视角
- 鼠标左键：射击
- `Esc`：退出

### Bloxorz 演示

- `W/A/S/D` 或方向键：翻转移动
- `R`：重置关卡
- `Esc`：退出

胜利后窗口标题会切换为完成提示，并可按 `R` 重新开始。

## 第三方依赖接入说明

### 已直接可用

- `gl3w`：位于 `third_party/gl3w`
- `stb_image`：位于 `third_party/stb`
- `miniaudio`：位于 `third_party/miniaudio`
- `sokol_app.h` / `sokol_gfx.h` / `sokol_glue.h` / `sokol_log.h`：位于 `third_party/sokol`
- `HandmadeMath.h`：位于 `third_party/handmade_math`
- `GLFW3` / `GLM`：由 CMake `FetchContent` 自动拉取（仅 FPS 原型使用）

### 可选库（自动探测）

CMake 会自动执行：

- `find_package(Jolt CONFIG QUIET)`
- `find_package(Effekseer CONFIG QUIET)`
- `find_package(nanogui CONFIG QUIET)`

当前仓库中已经把三者的**调用链与封装边界**搭好：

- `src/physics/PhysicsWorld.*`：Jolt `Character / CharacterVirtual` 的预留接入位置
- `src/rendering/ParticleSystem.*`：Effekseer 命中特效的预留接入位置
- `src/ui/HudLayer.*`：NanoGUI HUD 的预留接入位置

如果本地安装了这些库并导出对应 CMake package，工程会自动打开相应编译宏；如果没有安装，项目仍会以当前内置后备实现运行，避免原型结构混乱。

在无桌面图形环境的 CI / sandbox 中，FPS 工程会退化到 **headless gameplay self-check**：不创建窗口，但仍验证 XML 动画加载、固定步进、射击命中和得分主链路，方便持续集成验证。

对于 Bloxorz，本仓库额外提供 `bloxorz_logic_tests`，无需创建图形窗口即可覆盖：

- 三种方块姿态转换
- 合法/非法移动
- 动画结束后离散状态提交
- 完全站上目标格时的胜利判定

## XML 动画格式

`assets/enemy_anim.xml` 使用轻量格式定义帧 UV 源数据，例如：

```xml
<spritesheet texture="enemy_atlas.png" width="32" height="8">
  <animation name="idle" loop="true">
    <frame x="0" y="0" w="8" h="8" duration="0.10" />
  </animation>
</spritesheet>
```

`SpriteAnimationSet` 会把像素矩形转换为 UV，`EnemyActor` 在运行时推进帧并渲染到 3D quad 上。

## 当前验证结果

- `cmake -S . -B build-bloxorz`：通过
- `cmake --build build-bloxorz --target bloxorz_logic_tests -j`：通过
- `ctest --test-dir build-bloxorz --output-on-failure -R bloxorz_logic_tests`：通过
- 当前 sandbox 无法安装系统级 OpenGL/X11 开发包，且缺少 `GL/gl.h` 与 `X11/Xlib.h`，因此这里只完成了 `bloxorz_game` 的条件化 CMake 配置，未能在本环境实际编译或运行窗口目标

## 后续建议

- 将 `PhysicsWorld` 的后备运动逻辑替换为真正的 Jolt `CharacterVirtual`
- 在 `ParticleSystem` 中接入 Effekseer manager / effect asset
- 在 `HudLayer` 中接入真正的 NanoGUI widget 文本层
- 将兼容模式绘制替换为 VBO/VAO + shader 的完整 OpenGL 3 core 渲染路径
