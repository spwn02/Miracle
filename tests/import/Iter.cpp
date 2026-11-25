// Direct import only: view/borrow/category customizations and method-template bodies must be reachable
// from outside the library without the umbrella module (INV-ITER-003/004).
import std;
import Miracle.Iter;

namespace {

using Vector = std::vector<int>;
using Base = std::ranges::ref_view<Vector>;

// std::ranges customizations attached to every partition-owned type.
static_assert(std::ranges::view<Miracle::Iter<Base>>);
static_assert(std::ranges::borrowed_range<Miracle::Iter<Base>>);
static_assert(not std::ranges::borrowed_range<Miracle::Iter<Base, std::negate<>>>);
static_assert(std::ranges::view<Miracle::Cycle<Base>>);
static_assert(std::ranges::view<Miracle::Intersperse<Base>>);
static_assert(std::ranges::view<Miracle::ArrayChunks<Base, 2>>);
static_assert(std::ranges::view<Miracle::Peekable<Base>>);
static_assert(std::ranges::view<Miracle::RepeatWith<int (*)()>>);
static_assert(std::ranges::view<Miracle::FromFn<std::optional<int> (*)()>>);
static_assert(std::ranges::forward_range<Miracle::Iter<Base>>);
static_assert(std::ranges::random_access_range<Miracle::Iter<Base>>);

struct Point {
  int x;
  int y;
};

struct MoveOnlyAdd {
  constexpr MoveOnlyAdd() = default;
  constexpr MoveOnlyAdd(MoveOnlyAdd &&) = default;
  constexpr auto operator=(MoveOnlyAdd &&) -> MoveOnlyAdd & = default;
  constexpr auto operator()(int value) const -> int { return value + 1; }
};

// Custom user types and reflected-aggregate adaptive invocation (direct call -> tuple -> aggregate).
constexpr auto points() {
  std::array<Point, 3> values{{{1, 2}, {3, 4}, {5, 6}}};
  return Miracle::iter(values).map([](int x, int y) { return x * y; }).sum();
}
static_assert(points() == 1 * 2 + 3 * 4 + 5 * 6);

// Tuple decomposition.
constexpr auto tuples() {
  std::array<std::pair<int, int>, 2> values{{{2, 3}, {4, 5}}};
  return Miracle::iter(values).map([](int a, int b) { return a + b; }).sum();
}
static_assert(tuples() == 14);

// Move-only callable through rvalue pipeline, lvalue range and non-common sentinel source.
constexpr auto moveOnly() {
  std::array<int, 3> values{1, 2, 3};
  return Miracle::iter(values).map(MoveOnlyAdd{}).sum();
}
static_assert(moveOnly() == 9);

constexpr auto unbounded() {
  return Miracle::iter(std::views::iota(1)).map([](int value) { return value * 2; }).take(3).sum();
}
static_assert(unbounded() == 12);

// Source factories live in a different partition than Iter and the views they return.
static_assert(Miracle::once(4).sum() == 4);
static_assert(Miracle::repeatN(2, 3).sum() == 6);
static_assert(Miracle::empty<int>().sum() == 0);
static_assert(Miracle::successors(1, [](int v) { return v < 8 ? std::optional{v * 2} : std::nullopt; }).sum() ==
              1 + 2 + 4 + 8);

} // namespace

auto main() -> int {
  return 0;
}
