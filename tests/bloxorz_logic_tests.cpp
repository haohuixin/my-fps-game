#include <cstdlib>
#include <iostream>

#include "bloxorz/Game.h"

namespace {

void expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << "[bloxorz_logic_tests] " << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}

void advanceAnimation(bloxorz::Game& game) {
  expect(game.hasActiveAnimation(), "expected an active move animation");
  game.update(game.moveDurationSeconds());
  expect(!game.hasActiveAnimation(), "animation should finish after one duration step");
}

void testInitialState() {
  bloxorz::Game game;
  expect(game.block().pose == bloxorz::BlockPose::Standing, "initial block should stand upright");
  const auto cells = game.occupiedCells();
  expect(cells[0] == bloxorz::GridPos{2, 3}, "initial primary cell should match level start");
  expect(cells[1] == bloxorz::GridPos{2, 3}, "standing block should occupy one grid cell");
  expect(!game.won(), "initial state must not already be a win");
}

void testPoseTransitions() {
  bloxorz::Game game;
  expect(game.queueMove(bloxorz::MoveDirection::Right), "standing block should move right");
  expect(game.block().pose == bloxorz::BlockPose::Standing,
         "discrete state should not commit until animation completes");
  advanceAnimation(game);

  expect(game.block().pose == bloxorz::BlockPose::LyingX, "first right move should produce X-lying pose");
  auto cells = game.occupiedCells();
  expect(cells[0] == bloxorz::GridPos{3, 3} && cells[1] == bloxorz::GridPos{4, 3},
         "lying X pose should cover two adjacent X cells");

  expect(game.queueMove(bloxorz::MoveDirection::Up), "lying X block should move upward");
  advanceAnimation(game);
  expect(game.block().pose == bloxorz::BlockPose::LyingX,
         "moving sideways while lying on X should keep X pose");
  cells = game.occupiedCells();
  expect(cells[0] == bloxorz::GridPos{3, 2} && cells[1] == bloxorz::GridPos{4, 2},
         "lying X block should shift both occupied cells together");

  game.reset();
  expect(game.queueMove(bloxorz::MoveDirection::Up), "standing block should move up");
  advanceAnimation(game);
  expect(game.block().pose == bloxorz::BlockPose::LyingZ, "upward move should produce Z-lying pose");
  cells = game.occupiedCells();
  expect(cells[0] == bloxorz::GridPos{2, 1} && cells[1] == bloxorz::GridPos{2, 2},
         "lying Z pose should cover two adjacent Z cells");
}

void testIllegalMoveAndReset() {
  bloxorz::Game game;
  expect(!game.canMove(bloxorz::MoveDirection::Left), "initial left move should fall into void and be rejected");
  expect(!game.queueMove(bloxorz::MoveDirection::Left), "illegal move must not start animation");

  expect(game.queueMove(bloxorz::MoveDirection::Right), "legal move should start animation");
  advanceAnimation(game);
  game.reset();
  expect(game.block().pose == bloxorz::BlockPose::Standing, "reset should restore standing pose");
  const auto cells = game.occupiedCells();
  expect(cells[0] == bloxorz::GridPos{2, 3} && cells[1] == bloxorz::GridPos{2, 3},
         "reset should restore original occupied cell");
}

void testWinCondition() {
  bloxorz::Game game;
  expect(game.queueMove(bloxorz::MoveDirection::Right), "first solution step should be valid");
  advanceAnimation(game);
  expect(!game.won(), "lying across the goal should not count as clear");

  expect(game.queueMove(bloxorz::MoveDirection::Right), "second solution step should be valid");
  advanceAnimation(game);
  expect(game.won(), "standing completely on goal should win the level");
  expect(!game.canMove(bloxorz::MoveDirection::Down), "won state should not advertise further legal moves");
  expect(!game.queueMove(bloxorz::MoveDirection::Down), "won state should ignore further movement until reset");
}

}  // namespace

int main() {
  testInitialState();
  testPoseTransitions();
  testIllegalMoveAndReset();
  testWinCondition();
  std::cout << "bloxorz logic self-check passed\n";
  return EXIT_SUCCESS;
}
