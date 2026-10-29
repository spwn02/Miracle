import std;
import Miracle.Meta;

struct MoveOnly final {
  int value{};

  constexpr explicit MoveOnly(int input)
      : value(input) {
  }

  MoveOnly(const MoveOnly &) = delete;
  constexpr MoveOnly(MoveOnly &&) = default;
};

consteval auto moveOnlyBarrier() -> void {
  std::array values{1, 2};
  static_cast<void>(
      Miracle::meta::query(values).map([](int &value) consteval { return MoveOnly{value}; }).reverse());
}

consteval auto runMoveOnlyBarrier() -> bool {
  moveOnlyBarrier();
  return true;
}

static_assert(runMoveOnlyBarrier());
