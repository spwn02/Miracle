import Miracle.Meta;
import std;

consteval {
  (void)Miracle::meta::query(std::array{1, 2, 3});
}

auto main() -> int {
  return 0;
}
