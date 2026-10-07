import Miracle.Meta;
import std;

constexpr int values[]{1, 2, 3};
consteval {
  (void)Miracle::meta::query(std::span<const int>{values}).materialize();
}

auto main() -> int {
  return 0;
}
