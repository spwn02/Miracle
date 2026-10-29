import std;
import Miracle;

enum class[[= Miracle::bitflags]] Flags : unsigned { One = 1 };

template <std::size_t Size>
constexpr auto accepted(const char (&text)[Size], std::size_t length) -> bool {
  std::format_parse_context context(std::string_view{text, length});
  std::formatter<Flags> formatter;
  return formatter.parse(context) == context.end();
}

// These buffers have no hidden null terminator and no closing brace to mask a bounds error.
constexpr char binary[1] = {'b'};
constexpr char hex[1] = {'x'};
constexpr char prefix[1] = {'p'};
constexpr char binaryPrefix[2] = {'b', 'p'};
constexpr char hexPrefix[2] = {'x', 'p'};
static_assert(accepted(binary, 0));
static_assert(accepted(binary, 1));
static_assert(accepted(hex, 1));
static_assert(accepted(prefix, 1));
static_assert(accepted(binaryPrefix, 2));
static_assert(accepted(hexPrefix, 2));

enum class[[= Miracle::debug::derive]] Display : unsigned {
  First[[= Miracle::debug::prefer]] = 1,
  Hidden[[ = Miracle::debug::hide, = Miracle::debug::prefer ]] = 2,
  Last[[= Miracle::debug::prefer]] = 3,
  Alias = 3,
};
static_assert(Miracle::debug::enumName(Display::First) == "First");
static_assert(Miracle::debug::enumName(Display::Alias) == "Last");
static_assert(Miracle::debug::enumName(static_cast<Display>(0)) == "Last");

auto main() -> int {
  if (std::format("{:bp}", Flags::One) != "0b00000001" || std::format("{:xp}", Flags::One) != "0x1")
    return 1;
  for (std::string_view invalid : {"bx", "?"}) {
    bool rejected = false;
    try {
      std::format_parse_context context(invalid);
      std::formatter<Flags> formatter;
      formatter.parse(context);
    } catch (const std::format_error &) {
      rejected = true;
    }
    if (!rejected)
      return 2;
  }
  std::format_parse_context context("b}");
  std::formatter<Flags> formatter;
  if (*formatter.parse(context) != '}')
    return 3;
  return 0;
}
