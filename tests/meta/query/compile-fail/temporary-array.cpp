import std;
import Miracle.Meta;

consteval auto temporaryArrayQuery() {
  return Miracle::meta::query(std::array{1, 2, 3}).count();
}

static_assert(temporaryArrayQuery() == 3);
