import std;
import Miracle;
import Switch;

using namespace Miracle;
using namespace Switch;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
namespace Tests::reflection {

template <typename E>
concept Enum = std::is_enum_v<E>;

template <Enum E>
constexpr auto enum_to_string(E value) -> String { // NOLINT
  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template for (constexpr std::meta::info enumerator : meta::enumerators<^^E>) {
    if (value == [:enumerator:]) {
      return String(meta::identifier<enumerator>);
    }
  }
  return "<unnamed>";
}

enum class[[= debug::derive]] Color : u8 { red, green, blue };

[[
  = test,
  = Case{Color::red, "red"},
  = Case{Color::green, "green"},
  = Case{Color::blue, "blue"},
  = Case{Color{69}, "<unnamed>"},
  = group("experimental"),
  = tag("reflection")
]] auto enumToString(Color value, StringView expected) -> Expression {
  return eq(enum_to_string(value), expected);
}

template <typename T>
concept Struct = std::is_class_v<T>;

template <Struct T>
constexpr auto list_members(const T &val) -> String { // NOLINT
  Vec<String> buf{};

  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template for (constexpr auto mem : meta::nsMembers<^^T, meta::AccessContext::current()>) {
    buf.push_back(std::format("{}: {}", meta::identifier<mem>, val.[:mem:]));
  }

  return buf | std::views::join_with(StringView{"; "}) | std::ranges::to<String>();
}

struct[[= debug::derive]] Vec2 {
  u32 x{}, y{};
};

[[
  = test,
  = Case{Vec2{}, "x: 0; y: 0"},
  = Case{Vec2{.x = 69}, "x: 69; y: 0"},
  = Case{Vec2{.x = 34, .y = 35}, "x: 34; y: 35"},
  = group("experimental"),
  = tag("reflection")
]] auto listMembers(Vec2 vec2, StringView expected) -> Expression {
  return eq(list_members(vec2), expected);
}

} // namespace Tests::reflection
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  discover<^^Tests::reflection>(recursive);
}
