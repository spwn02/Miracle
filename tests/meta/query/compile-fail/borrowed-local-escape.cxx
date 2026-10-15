import std;
import Miracle.Meta;

consteval auto borrowedLocalEscape() -> std::size_t {
  std::array outer{1};
  auto nested = Miracle::meta::query(outer).flatMap([](int &) consteval {
    std::array local{2, 3};
    return Miracle::meta::query(local);
  });
  return nested.count();
}

static_assert(borrowedLocalEscape() == 2);
