import Miracle.Meta;
import std;

inline constexpr std::array source{1};

consteval auto escapedNestedQuery(int value) {
  std::array local{value, value + 1};
  return Miracle::meta::query(local);
}

constexpr auto dangling =
    Miracle::meta::query(source).flatMap([](int value) consteval { return escapedNestedQuery(value); });

static_assert(dangling.count() == 2);
