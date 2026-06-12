# my-fps-game

一个最小可运行的 FPS 原型（OpenGL3/GLFW3/GLM/gl3w/stb_image/miniaudio），并预留了 Jolt / Effekseer / NanoGUI 的接入点（通过 CMake 自动探测目标库）。

## 构建

```bash
cmake -S . -B build
cmake --build build -j
```

## 运行

```bash
./build/fps_game
```

## 操作

- `W/A/S/D`: 移动
- `Space`: 跳跃
- 鼠标移动: 视角 yaw/pitch（pitch 已限制）
- 鼠标左键: 射击

## 当前原型功能

- 第一人称相机
- WASD + 跳跃
- 2D 贴图敌人（billboard quad）
- 命中判定与得分累加
- 命中屏幕闪烁反馈（Effekseer 预留接入点）
- 射击音效（miniaudio）
- 分数显示（窗口标题；NanoGUI 预留接入点）
