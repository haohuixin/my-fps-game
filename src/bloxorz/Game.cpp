#include "bloxorz/Game.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace bloxorz {
namespace {
constexpr float kPiOverTwo = 1.57079632679f;

[[nodiscard]] float smoothstep(float t) {
  t = std::clamp(t, 0.0f, 1.0f);
  return t * t * (3.0f - 2.0f * t);
}
}  // namespace

TileType Level::tileAt(GridPos pos) const {
  if (pos.x < 0 || pos.z < 0 || pos.x >= width || pos.z >= height) {
    return TileType::Void;
  }
  return tiles[static_cast<std::size_t>(pos.z * width + pos.x)];
}

bool Level::isSolid(GridPos pos) const {
  const TileType tile = tileAt(pos);
  return tile == TileType::Floor || tile == TileType::Goal;
}

Game::Game() : level_(createDefaultLevel()), block_(level_.start) {
  updateWinState();
}

std::array<GridPos, 2> Game::occupiedCells() const {
  return occupiedCellsFor(block_);
}

bool Game::canMove(MoveDirection direction) const {
  return nextState(direction).has_value();
}

bool Game::queueMove(MoveDirection direction) {
  if (won_ || animation_.has_value()) {
    return false;
  }

  const std::optional<BlockState> candidate = nextState(direction);
  if (!candidate.has_value()) {
    return false;
  }

  animation_ = MoveAnimation{
      .start = block_,
      .finish = *candidate,
      .direction = direction,
      .pivot = pivotForMove(block_, direction),
      .axis = axisForMove(direction),
      .elapsed = 0.0f,
      .duration = kMoveDurationSeconds,
      .startAngle = 0.0f,
      .deltaAngle = signedQuarterTurn(direction),
  };
  return true;
}

void Game::update(float dt) {
  if (!animation_.has_value()) {
    return;
  }

  animation_->elapsed += std::max(dt, 0.0f);
  if (animation_->elapsed + 1e-6f < animation_->duration) {
    return;
  }

  block_ = animation_->finish;
  animation_.reset();
  updateWinState();
}

void Game::reset() {
  block_ = level_.start;
  animation_.reset();
  won_ = false;
  updateWinState();
}

HMM_Mat4 Game::blockModelMatrix() const {
  if (!animation_.has_value()) {
    return blockMatrixForState(block_);
  }

  const float t = smoothstep(animation_->elapsed / animation_->duration);
  const float angle = animation_->startAngle + animation_->deltaAngle * t;
  const HMM_Mat4 aroundPivot = HMM_MulM4(
      HMM_MulM4(HMM_Translate(animation_->pivot), HMM_Rotate_RH(angle, animation_->axis)),
      HMM_Translate(HMM_MulV3F(animation_->pivot, -1.0f)));
  return HMM_MulM4(aroundPivot, blockMatrixForState(animation_->start));
}

HMM_Mat4 Game::tileModelMatrix(GridPos pos, TileType tile) {
  if (tile == TileType::Void) {
    return HMM_M4D(1.0f);
  }

  const HMM_Vec3 center = HMM_V3(static_cast<float>(pos.x), -kTileHeight * 0.5f,
                                 static_cast<float>(pos.z));
  return HMM_MulM4(HMM_Translate(center), HMM_Scale(HMM_V3(0.96f, kTileHeight, 0.96f)));
}

Level Game::createDefaultLevel() {
  static constexpr const char* rows[] = {
      "       ",
      " ..... ",
      " ..... ",
      " .S..G ",
      " ..... ",
      " ..... ",
      "       ",
  };

  Level level;
  level.name = "Starter Bridge";
  level.height = static_cast<int>(std::size(rows));
  level.width = static_cast<int>(std::char_traits<char>::length(rows[0]));
  level.tiles.resize(static_cast<std::size_t>(level.width * level.height), TileType::Void);

  for (int z = 0; z < level.height; ++z) {
    for (int x = 0; x < level.width; ++x) {
      const char cell = rows[z][x];
      const std::size_t index = static_cast<std::size_t>(z * level.width + x);
      switch (cell) {
        case '.':
          level.tiles[index] = TileType::Floor;
          break;
        case 'G':
          level.tiles[index] = TileType::Goal;
          break;
        case 'S':
          level.tiles[index] = TileType::Floor;
          level.start = BlockState{.pose = BlockPose::Standing,
                                   .primary = GridPos{x, z},
                                   .secondary = GridPos{x, z}};
          break;
        default:
          level.tiles[index] = TileType::Void;
          break;
      }
    }
  }
  return level;
}

BlockState Game::canonicalize(BlockState state) {
  if (state.pose == BlockPose::Standing) {
    state.secondary = state.primary;
    return state;
  }
  if (state.pose == BlockPose::LyingX && state.secondary.x < state.primary.x) {
    std::swap(state.primary, state.secondary);
  }
  if (state.pose == BlockPose::LyingZ && state.secondary.z < state.primary.z) {
    std::swap(state.primary, state.secondary);
  }
  return state;
}

std::array<GridPos, 2> Game::occupiedCellsFor(const BlockState& state) {
  const BlockState canonical = canonicalize(state);
  return {canonical.primary, canonical.secondary};
}

HMM_Vec3 Game::centerFor(const BlockState& state) {
  const BlockState canonical = canonicalize(state);
  switch (canonical.pose) {
    case BlockPose::Standing:
      return HMM_V3(static_cast<float>(canonical.primary.x), 1.0f, static_cast<float>(canonical.primary.z));
    case BlockPose::LyingX:
      return HMM_V3(static_cast<float>(canonical.primary.x + canonical.secondary.x) * 0.5f, 0.5f,
                    static_cast<float>(canonical.primary.z));
    case BlockPose::LyingZ:
      return HMM_V3(static_cast<float>(canonical.primary.x), 0.5f,
                    static_cast<float>(canonical.primary.z + canonical.secondary.z) * 0.5f);
  }
  throw std::logic_error("Unhandled block pose");
}

HMM_Mat4 Game::poseRotation(BlockPose pose) {
  switch (pose) {
    case BlockPose::Standing:
      return HMM_M4D(1.0f);
    case BlockPose::LyingX:
      return HMM_Rotate_RH(kPiOverTwo, HMM_V3(0.0f, 0.0f, 1.0f));
    case BlockPose::LyingZ:
      return HMM_Rotate_RH(kPiOverTwo, HMM_V3(1.0f, 0.0f, 0.0f));
  }
  throw std::logic_error("Unhandled block pose");
}

HMM_Mat4 Game::baseBlockScale() {
  return HMM_Scale(HMM_V3(kBlockWidth, kBlockHeight, kBlockWidth));
}

HMM_Mat4 Game::blockMatrixForState(const BlockState& state) {
  return HMM_MulM4(HMM_MulM4(HMM_Translate(centerFor(state)), poseRotation(state.pose)), baseBlockScale());
}

HMM_Vec3 Game::pivotForMove(const BlockState& state, MoveDirection direction) {
  const auto cells = occupiedCellsFor(state);
  const int minX = std::min(cells[0].x, cells[1].x);
  const int maxX = std::max(cells[0].x, cells[1].x);
  const int minZ = std::min(cells[0].z, cells[1].z);
  const int maxZ = std::max(cells[0].z, cells[1].z);
  const float avgX = static_cast<float>(minX + maxX) * 0.5f;
  const float avgZ = static_cast<float>(minZ + maxZ) * 0.5f;

  switch (direction) {
    case MoveDirection::Left:
      return HMM_V3(static_cast<float>(minX) - 0.5f, 0.0f, avgZ);
    case MoveDirection::Right:
      return HMM_V3(static_cast<float>(maxX) + 0.5f, 0.0f, avgZ);
    case MoveDirection::Up:
      return HMM_V3(avgX, 0.0f, static_cast<float>(minZ) - 0.5f);
    case MoveDirection::Down:
      return HMM_V3(avgX, 0.0f, static_cast<float>(maxZ) + 0.5f);
  }
  throw std::logic_error("Unhandled move direction");
}

HMM_Vec3 Game::axisForMove(MoveDirection direction) {
  switch (direction) {
    case MoveDirection::Left:
    case MoveDirection::Right:
      return HMM_V3(0.0f, 0.0f, 1.0f);
    case MoveDirection::Up:
    case MoveDirection::Down:
      return HMM_V3(1.0f, 0.0f, 0.0f);
  }
  throw std::logic_error("Unhandled move direction");
}

float Game::signedQuarterTurn(MoveDirection direction) {
  switch (direction) {
    case MoveDirection::Left:
      return kPiOverTwo;
    case MoveDirection::Right:
      return -kPiOverTwo;
    case MoveDirection::Up:
      return -kPiOverTwo;
    case MoveDirection::Down:
      return kPiOverTwo;
  }
  throw std::logic_error("Unhandled move direction");
}

std::optional<BlockState> Game::nextState(MoveDirection direction) const {
  BlockState candidate = block_;
  const BlockState canonical = canonicalize(block_);

  switch (canonical.pose) {
    case BlockPose::Standing:
      switch (direction) {
        case MoveDirection::Left:
          candidate = BlockState{.pose = BlockPose::LyingX,
                                 .primary = GridPos{canonical.primary.x - 2, canonical.primary.z},
                                 .secondary = GridPos{canonical.primary.x - 1, canonical.primary.z}};
          break;
        case MoveDirection::Right:
          candidate = BlockState{.pose = BlockPose::LyingX,
                                 .primary = GridPos{canonical.primary.x + 1, canonical.primary.z},
                                 .secondary = GridPos{canonical.primary.x + 2, canonical.primary.z}};
          break;
        case MoveDirection::Up:
          candidate = BlockState{.pose = BlockPose::LyingZ,
                                 .primary = GridPos{canonical.primary.x, canonical.primary.z - 2},
                                 .secondary = GridPos{canonical.primary.x, canonical.primary.z - 1}};
          break;
        case MoveDirection::Down:
          candidate = BlockState{.pose = BlockPose::LyingZ,
                                 .primary = GridPos{canonical.primary.x, canonical.primary.z + 1},
                                 .secondary = GridPos{canonical.primary.x, canonical.primary.z + 2}};
          break;
      }
      break;
    case BlockPose::LyingX:
      switch (direction) {
        case MoveDirection::Left:
          candidate = BlockState{.pose = BlockPose::Standing,
                                 .primary = GridPos{canonical.primary.x - 1, canonical.primary.z},
                                 .secondary = GridPos{canonical.primary.x - 1, canonical.primary.z}};
          break;
        case MoveDirection::Right:
          candidate = BlockState{.pose = BlockPose::Standing,
                                 .primary = GridPos{canonical.secondary.x + 1, canonical.primary.z},
                                 .secondary = GridPos{canonical.secondary.x + 1, canonical.primary.z}};
          break;
        case MoveDirection::Up:
          candidate = BlockState{.pose = BlockPose::LyingX,
                                 .primary = GridPos{canonical.primary.x, canonical.primary.z - 1},
                                 .secondary = GridPos{canonical.secondary.x, canonical.secondary.z - 1}};
          break;
        case MoveDirection::Down:
          candidate = BlockState{.pose = BlockPose::LyingX,
                                 .primary = GridPos{canonical.primary.x, canonical.primary.z + 1},
                                 .secondary = GridPos{canonical.secondary.x, canonical.secondary.z + 1}};
          break;
      }
      break;
    case BlockPose::LyingZ:
      switch (direction) {
        case MoveDirection::Left:
          candidate = BlockState{.pose = BlockPose::LyingZ,
                                 .primary = GridPos{canonical.primary.x - 1, canonical.primary.z},
                                 .secondary = GridPos{canonical.secondary.x - 1, canonical.secondary.z}};
          break;
        case MoveDirection::Right:
          candidate = BlockState{.pose = BlockPose::LyingZ,
                                 .primary = GridPos{canonical.primary.x + 1, canonical.primary.z},
                                 .secondary = GridPos{canonical.secondary.x + 1, canonical.secondary.z}};
          break;
        case MoveDirection::Up:
          candidate = BlockState{.pose = BlockPose::Standing,
                                 .primary = GridPos{canonical.primary.x, canonical.primary.z - 1},
                                 .secondary = GridPos{canonical.primary.x, canonical.primary.z - 1}};
          break;
        case MoveDirection::Down:
          candidate = BlockState{.pose = BlockPose::Standing,
                                 .primary = GridPos{canonical.secondary.x, canonical.secondary.z + 1},
                                 .secondary = GridPos{canonical.secondary.x, canonical.secondary.z + 1}};
          break;
      }
      break;
  }

  candidate = canonicalize(candidate);
  if (!isStateSupported(candidate)) {
    return std::nullopt;
  }
  return candidate;
}

bool Game::isStateSupported(const BlockState& state) const {
  const auto cells = occupiedCellsFor(state);
  return level_.isSolid(cells[0]) && level_.isSolid(cells[1]);
}

void Game::updateWinState() {
  won_ = false;
  if (block_.pose != BlockPose::Standing) {
    return;
  }
  won_ = level_.tileAt(block_.primary) == TileType::Goal;
}

}  // namespace bloxorz
