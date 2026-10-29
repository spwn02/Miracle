import std;
import Miracle;

using namespace Miracle;

namespace {

constexpr auto isEven(i32 value) noexcept -> bool {
  return value % 2 == 0;
}

constexpr auto square(i32 value) noexcept -> i32 {
  return value * value;
}

} // namespace

auto main() -> int { // NOLINT
  const Vec<i32> values{1, 2, 3, 4, 5, 6, 7, 8};
  const Vec<i32> result = iter(values).filter(isEven).map(square).take(3).toVec();

  for (i32 value : result) {
    std::println("{}", value);
  }
}
