import Miracle.Meta;
import std;

struct MoveOnlyValue final {
  int value{};

  consteval explicit MoveOnlyValue(int input)
      : value(input) {
  }

  MoveOnlyValue(const MoveOnlyValue &) = delete;
  consteval MoveOnlyValue(MoveOnlyValue &&other)
      : value(other.value) {
  }
};

constexpr std::array source{1, 2, 3};

consteval {
  auto values =
      Miracle::meta::query(source).map([](const int &value) consteval { return MoveOnlyValue{value}; });
  (void)std::move(values).reverse();
}

auto main() -> int {
  return 0;
}
