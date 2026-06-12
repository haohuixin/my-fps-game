# my-fps-game

一个可运行的 C++ FPS 原型，按 `core / rendering / physics / gameplay / audio / ui` 模块拆分，使用并预留以下技术栈接入点：

- OpenGL 3.3（兼容模式上下文，便于原型阶段快速渲染）
- GLFW3
- GLM
- gl3w
- stb_image
- miniaudio
- Jolt Physics（可选接入点）
- Effekseer（可选接入点）
- nanogui（可选接入点）

## 当前原型内容

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

## 目录结构

```text
src/
  core/        应用主循环、窗口、固定步进
  rendering/   纹理、精灵动画、粒子
  physics/     玩家/敌人碰撞与射线查询封装（Jolt 接入点）
  gameplay/    相机、玩家控制、武器、敌人
  audio/       miniaudio 封装
  ui/          HUD 层
assets/
  enemy_atlas.png
  enemy_anim.xml
  shoot.wav
third_party/
  gl3w/
  stb/
  miniaudio/
```

## 构建

Linux 下建议先安装基础 OpenGL/X11 开发包：

```bash
sudo apt-get update
sudo apt-get install -y libgl1-mesa-dev xorg-dev
```

然后在仓库根目录执行：

```bash
cmake -S . -B build
cmake --build build -j
```

## 运行

```bash
./build/fps_game
```

## 操作

- `W/A/S/D`：移动
- `Space`：跳跃
- 鼠标移动：第一人称视角
- 鼠标左键：射击
- `Esc`：退出

## 第三方依赖接入说明

### 已直接可用

- `gl3w`：位于 `third_party/gl3w`
- `stb_image`：位于 `third_party/stb`
- `miniaudio`：位于 `third_party/miniaudio`
- `GLFW3` / `GLM`：由 CMake `FetchContent` 自动拉取

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

## 后续建议

- 将 `PhysicsWorld` 的后备运动逻辑替换为真正的 Jolt `CharacterVirtual`
- 在 `ParticleSystem` 中接入 Effekseer manager / effect asset
- 在 `HudLayer` 中接入真正的 NanoGUI widget 文本层
- 将兼容模式绘制替换为 VBO/VAO + shader 的完整 OpenGL 3 core 渲染路径
