import std;
import Miracle.Meta;
consteval bool test() {
  std::array nested{std::array{1, 2}, std::array{3, 4}};
  auto q =
      Miracle::meta::query(nested).map([](auto &r) consteval -> auto && { return std::move(r); }).flatten();
  static_assert(std::same_as<typename decltype(q)::Value, int>);
  const auto values = q.materialize();
  return values.size() == 4 && values[0] == 1 && values[1] == 2 && values[2] == 3 && values[3] == 4;
}
static_assert(test());

auto main() -> int {
}
