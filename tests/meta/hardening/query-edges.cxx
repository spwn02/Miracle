import std;
import Miracle.Meta;
using Miracle::meta::query;

consteval auto boundaries() -> bool {
  std::array values{1, 2, 3, 4};
  std::size_t observed{};
  if (query(values).inspect([&](int &) consteval { ++observed; }).take(0).count() != 0 || observed != 1)
    return false;
  if (query(values).skip(2).take(2).count() != 2)
    return false;
  if (query(values).slice(1, 3).skip(2).count() != 0)
    return false;
  if (query(values).skip(std::numeric_limits<std::size_t>::max()).skip(1).count() != 0)
    return false;
  if (query(values)
          .skipWhile([](int n) consteval { return n < 2; })
          .takeWhile([](int n) consteval { return n < 4; })
          .count() != 2)
    return false;
  std::array<int, 0> empty{};
  return query(empty).isEmpty() && query(empty).any([](int) consteval { return true; }) == false &&
         query(empty).all([](int) consteval { return false; }) && query(empty).reverse().count() == 0;
}
static_assert(boundaries());

struct MoveOnly final {
  int value;
  constexpr explicit MoveOnly(int n)
      : value(n) {
  }
  MoveOnly(const MoveOnly &) = delete;
  constexpr MoveOnly(MoveOnly &&) = default;
};
struct Immovable final {
  int value;
  constexpr explicit Immovable(int n)
      : value(n) {
  }
  Immovable(const Immovable &) = delete;
  Immovable(Immovable &&) = delete;
};
consteval auto lifetimes() -> bool {
  std::array input{1, 2};
  int total{};
  query(input)
      .map([](int n) consteval { return MoveOnly{n}; })
      .map([](MoveOnly n) consteval { return Immovable{n.value}; })
      .map([](Immovable &&n) consteval { return Immovable{n.value + 1}; })
      .forEach([&](const Immovable &n) consteval { total += n.value; });
  auto captured =
      query(input).map([state = std::make_unique<int>(2)](int n) consteval { return n + *state; });
  return total == 5 && std::move(captured).count() == 2;
}
static_assert(lifetimes());

struct Tracked final {
  int *alive;
  constexpr explicit Tracked(int &count)
      : alive(&count) {
    ++*alive;
  }
  Tracked(const Tracked &) = delete;
  Tracked(Tracked &&) = delete;
  constexpr ~Tracked() {
    --*alive;
  }
};
consteval auto unwinding() -> bool {
  std::array input{1};
  int alive{};
  try {
    query(input)
        .map([&](int) consteval { return Tracked{alive}; })
        .map([](Tracked &&value) consteval { return Tracked{*value.alive}; })
        .inspect([](const Tracked &) consteval -> void { throw 17; })
        .count();
  } catch (int value) {
    return value == 17 && alive == 0;
  }
  return false;
}
static_assert(unwinding());
auto main() -> int {
}
