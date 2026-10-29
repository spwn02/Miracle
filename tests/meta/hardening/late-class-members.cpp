import std;
import Miracle.Meta;
struct Late {
  auto member();
};
constexpr auto rawBefore = std::meta::members_of(^^Late, std::meta::access_context::unchecked()).size();
constexpr auto facadeBefore = Miracle::meta::members<Late>(Miracle::Access::unchecked()).size();
static_assert(facadeBefore == rawBefore);
constexpr auto functionsBefore = Miracle::meta::functions<Late>(Miracle::Access::unchecked()).size();
constexpr auto reflectedBefore = Miracle::reflect<Late>().members(Miracle::Access::unchecked()).size();
static_assert(reflectedBefore == rawBefore);
auto Late::member() {
  return 1;
}
constexpr auto rawAfter = std::meta::members_of(^^Late, std::meta::access_context::unchecked()).size();
static_assert(rawAfter > rawBefore);
static_assert(Miracle::meta::members<Late>(Miracle::Access::unchecked()).size() == rawAfter);
static_assert(Miracle::reflect<Late>().members(Miracle::Access::unchecked()).size() == rawAfter);
static_assert(Miracle::meta::functions<Late>(Miracle::Access::unchecked()).size() == functionsBefore + 1);
static_assert(Miracle::reflect<Late>().functions(Miracle::Access::unchecked()).size() == functionsBefore + 1);
static_assert(std::ranges::equal(Miracle::meta::members(^^Late, Miracle::Access::unchecked()),
    std::meta::members_of(^^Late, Miracle::Access::unchecked())));

auto main() -> int {
}
