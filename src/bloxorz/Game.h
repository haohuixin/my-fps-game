#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "HandmadeMath.h"

namespace bloxorz {

enum class TileType {
  Void,
  Floor,
  Goal,
};

enum class MoveDirection {
  Left,
  Right,
  Up,
  Down,
};

enum class BlockPose {
  Standing,
  LyingX,
  LyingZ,
};

struct GridPos {
  int x = 0;
  int z = 0;

  [[nodiscard]] bool operator==(const GridPos& other) const = default;
};

struct BlockState {
  BlockPose pose = BlockPose::Standing;
  GridPos primary{};
  GridPos secondary{};
};

struct MoveAnimation {
  BlockState start{};
  BlockState finish{};
  MoveDirection direction = MoveDirection::Right;
  HMM_Vec3 pivot = HMM_V3(0.0f, 0.0f, 0.0f);
  HMM_Vec3 axis = HMM_V3(0.0f, 1.0f, 0.0f);
  float elapsed = 0.0f;
  float duration = 0.2f;
  float startAngle = 0.0f;
  float deltaAngle = 0.0f;
};

struct Level {
  std::string name;
  int width = 0;
  int height = 0;
  std::vector<TileType> tiles;
  BlockState start{};

  [[nodiscard]] TileType tileAt(GridPos pos) const;
  [[nodiscard]] bool isSolid(GridPos pos) const;
};

class Game {
 public:
  Game();

  [[nodiscard]] const Level& level() const { return level_; }
  [[nodiscard]] const BlockState& block() const { return block_; }
  [[nodiscard]] std::array<GridPos, 2> occupiedCells() const;
  [[nodiscard]] bool hasActiveAnimation() const { return animation_.has_value(); }
  [[nodiscard]] const std::optional<MoveAnimation>& animation() const { return animation_; }
  [[nodiscard]] bool won() const { return won_; }
  [[nodiscard]] float moveDurationSeconds() const { return kMoveDurationSeconds; }

  [[nodiscard]] bool canMove(MoveDirection direction) const;
  bool queueMove(MoveDirection direction);
  void update(float dt);
  void reset();

  [[nodiscard]] HMM_Mat4 blockModelMatrix() const;
  [[nodiscard]] static HMM_Mat4 tileModelMatrix(GridPos pos, TileType tile);

 private:
  static constexpr float kMoveDurationSeconds = 0.2f;
  static constexpr float kBlockWidth = 0.88f;
  static constexpr float kBlockHeight = 1.88f;
  static constexpr float kTileHeight = 0.14f;

  Level level_;
  BlockState block_{};
  std::optional<MoveAnimation> animation_;
  bool won_ = false;

  [[nodiscard]] static Level createDefaultLevel();
  [[nodiscard]] static BlockState canonicalize(BlockState state);
  [[nodiscard]] static std::array<GridPos, 2> occupiedCellsFor(const BlockState& state);
  [[nodiscard]] static HMM_Vec3 centerFor(const BlockState& state);
  [[nodiscard]] static HMM_Mat4 poseRotation(BlockPose pose);
  [[nodiscard]] static HMM_Mat4 baseBlockScale();
  [[nodiscard]] static HMM_Mat4 blockMatrixForState(const BlockState& state);
  [[nodiscard]] static HMM_Vec3 pivotForMove(const BlockState& state, MoveDirection direction);
  [[nodiscard]] static HMM_Vec3 axisForMove(MoveDirection direction);
  [[nodiscard]] static float signedQuarterTurn(MoveDirection direction);
  [[nodiscard]] std::optional<BlockState> nextState(MoveDirection direction) const;
  [[nodiscard]] bool isStateSupported(const BlockState& state) const;
  void updateWinState();
};

}  // namespace bloxorz
