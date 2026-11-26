import std;
import Miracle.Meta;

struct NonStructural final {
  constexpr explicit NonStructural(int input)
      : value_(input) {
  }

  [[nodiscard]] constexpr auto value() const -> int {
    return value_;
  }

private:
  int value_{};
};

consteval auto nonStructuralBarrier() -> void {
  std::array values{1, 2};
  static_cast<void>(
      Miracle::meta::query(values).map([](int &value) consteval { return NonStructural{value}; }).reverse());
}

consteval auto runNonStructuralBarrier() -> bool {
  nonStructuralBarrier();
  return true;
}

static_assert(runNonStructuralBarrier());
