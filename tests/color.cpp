import std;

import Switch;

import Miracle;

using namespace Miracle;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
namespace {

// checkedRgb accepts any non-bool integral type and rejects every out-of-range channel.
static_assert(Color::checkedRgb(0, 255, 128) == Color::rgb(0, 255, 128));
static_assert(not Color::checkedRgb(-1, 0, 0));
static_assert(not Color::checkedRgb(0, 256, 0));
static_assert(not Color::checkedRgb(0, 0, std::numeric_limits<int>::min()));
static_assert(not Color::checkedRgb(0, 0, std::numeric_limits<std::uint64_t>::max()));
static_assert(Color::checkedRgb(std::uint64_t{255}, std::int8_t{0}, 7L).has_value());

template <class... Channels>
concept CheckedRgbAccepts = requires(Channels... channels) { Color::checkedRgb(channels...); };
static_assert(CheckedRgbAccepts<int, int, int>);
static_assert(not CheckedRgbAccepts<bool, int, int>);
static_assert(not CheckedRgbAccepts<int, bool, int>);
static_assert(not CheckedRgbAccepts<int, int, bool>);
static_assert(not CheckedRgbAccepts<double, int, int>);

// hex: only 24 bits are valid.
static_assert(Color::hex(0x000000) == Color::rgb(0, 0, 0));
static_assert(Color::hex(0xFF9F43) == Color::rgb(255, 159, 67));
static_assert(not Color::hex(0x1000000));
static_assert(not Color::hex(0xFF000000));
static_assert(Color::rgb(255, 159, 67).toHex() == 0xFF9F43U);

// parseHex: exactly #RGB or #RRGGBB, ASCII hex digits, case-insensitive.
static_assert(Color::parseHex("#F00FFF") == Color::rgb(240, 15, 255));
static_assert(Color::parseHex("#abc") == Color::rgb(0xAA, 0xBB, 0xCC));
static_assert(Color::parseHex("#ABC") == Color::rgb(0xAA, 0xBB, 0xCC));
static_assert(Color::parseHex("#FF0") == Color::rgb(255, 255, 0));
static_assert(Color::parseHex("#000") == Color::rgb(0, 0, 0));
static_assert(Color::parseHex("#fFfFfF") == Color::rgb(255, 255, 255));
static_assert(not Color::parseHex(""));
static_assert(not Color::parseHex("#"));
static_assert(not Color::parseHex("FF0000"));
static_assert(not Color::parseHex("fff"));
static_assert(not Color::parseHex("#ff"));
static_assert(not Color::parseHex("#ffff"));
static_assert(not Color::parseHex("#fffff"));
static_assert(not Color::parseHex("#ffffff0"));
static_assert(not Color::parseHex("#ffg"));
static_assert(not Color::parseHex("#12345g"));
static_assert(not Color::parseHex("#0x1234"));
static_assert(not Color::parseHex(" #fff"));
static_assert(not Color::parseHex("#ff f"));
static_assert(not Color::parseHex("#-12"));
static_assert(not Color::parseHex("#+12"));
static_assert(not Color::parseHex(std::string_view{"#f\0f", 4}));
static_assert(not Color::parseHex("#\xC3\xA9\xC3\xA9"));

// HSV: sectors, grayscale, boundaries and rejection of invalid input.
static_assert(Color::fromHsv({0, 1, 1}) == Color::rgb(255, 0, 0));
static_assert(Color::fromHsv({60, 1, 1}) == Color::rgb(255, 255, 0));
static_assert(Color::fromHsv({120, 1, 1}) == Color::rgb(0, 255, 0));
static_assert(Color::fromHsv({180, 1, 1}) == Color::rgb(0, 255, 255));
static_assert(Color::fromHsv({240, 1, 1}) == Color::rgb(0, 0, 255));
static_assert(Color::fromHsv({300, 1, 1}) == Color::rgb(255, 0, 255));
static_assert(Color::fromHsv({359.999, 1, 1}) == Color::rgb(255, 0, 0));
static_assert(Color::fromHsv({200, 0, 0.5}) == Color::rgb(128, 128, 128));
static_assert(Color::fromHsv({0, 0, 0}) == Color::rgb(0, 0, 0));
static_assert(Color::fromHsv({0, 0, 1}) == Color::rgb(255, 255, 255));
static_assert(not Color::fromHsv({360, 1, 1}));
static_assert(not Color::fromHsv({-0.001, 1, 1}));
static_assert(not Color::fromHsv({0, 1.001, 1}));
static_assert(not Color::fromHsv({0, -0.001, 1}));
static_assert(not Color::fromHsv({0, 1, 1.001}));
static_assert(not Color::fromHsv({0, 1, -0.001}));
static_assert(not Color::fromHsv({std::numeric_limits<f64>::quiet_NaN(), 1, 1}));
static_assert(not Color::fromHsv({0, std::numeric_limits<f64>::quiet_NaN(), 1}));
static_assert(not Color::fromHsv({0, 1, std::numeric_limits<f64>::infinity()}));
static_assert(not Color::fromHsv({std::numeric_limits<f64>::infinity(), 1, 1}));

static_assert(Color::rgb(0, 0, 0).toHsv() == Hsv{0, 0, 0});
static_assert(Color::rgb(255, 255, 255).toHsv() == Hsv{0, 0, 1});
static_assert(Color::rgb(128, 128, 128).toHsv().hueDegrees == 0);
static_assert(Color::rgb(255, 0, 0).toHsv() == Hsv{0, 1, 1});
static_assert(Color::rgb(0, 255, 0).toHsv() == Hsv{120, 1, 1});
static_assert(Color::rgb(0, 0, 255).toHsv() == Hsv{240, 1, 1});
static_assert(Color::rgb(255, 0, 255).toHsv().hueDegrees == 300);
static_assert(Color::rgb(255, 0, 1).toHsv().hueDegrees < 360);

// Default is black; the type is a structural, trivially copyable value.
static_assert(Color{} == Color::rgb(0, 0, 0));
static_assert(std::is_trivially_copyable_v<Color> and std::is_standard_layout_v<Color>);
static_assert(sizeof(Color) == 3);

consteval auto roundTripsAllChannels() -> bool {
  for (u32 value{}; value <= 0xFFFFFF; value += 0x010101U * 5) {
    const Color color = *Color::hex(value);
    const auto back = Color::fromHsv(color.toHsv());
    if (not back)
      return false;
    const auto near = [](u8 left, u8 right) { return left > right ? left - right <= 1 : right - left <= 1; };
    if (not near(back->red, color.red) or not near(back->green, color.green) or not near(back->blue, color.blue))
      return false;
  }
  return true;
}
static_assert(roundTripsAllChannels());

} // namespace

namespace Tests::color {

[[= Switch::test]] auto parseHexRuntime() -> void {
  Switch::check(Color::parseHex("#F00FFF") == Color::rgb(240, 15, 255));
  Switch::check(Color::parseHex("#FF0") == Color::rgb(255, 255, 0));
  Switch::check(Color::parseHex("#FF0")->toHex() == 0xFFFF00U);
  Switch::check(not Color::parseHex("FF0"));
}

[[= Switch::test]] auto hsvRoundTripRuntime() -> void {
  constexpr Hsv red{.hueDegrees = 0, .saturation = 1, .value = 1};
  Switch::check(Color::fromHsv(red)->toHsv() == red);
  Switch::check(Color::fromHsv(red)->toHex() == 0xFF0000U);
}

} // namespace Tests::color
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  Switch::discover<^^Tests::color>();
}
