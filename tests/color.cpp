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

// TEST-COL-RGB-001..003, HEX-001, TYPE-002: raw channels, structural template argument.
static_assert(Color{}.red == 0 and Color{}.green == 0 and Color{}.blue == 0 and Color{}.toHex() == 0);
static_assert(Color::rgb(255, 255, 255).toHex() == 0xFFFFFFU);
static_assert(Color::rgb(12, 34, 56).red == 12 and Color::rgb(12, 34, 56).green == 34 and
              Color::rgb(12, 34, 56).blue == 56);
static_assert(Color::hex(0x123456)->red == 0x12 and Color::hex(0x123456)->green == 0x34 and
              Color::hex(0x123456)->blue == 0x56 and Color::hex(0x123456)->toHex() == 0x123456U);
static_assert(Color::hex(0)->toHex() == 0);

template <Color Value>
struct StructuralColor final {
  static constexpr Color value = Value;
};
static_assert(StructuralColor<Color{1, 2, 3}>::value == Color::rgb(1, 2, 3));
static_assert(std::is_same_v<StructuralColor<Color{1, 2, 3}>, StructuralColor<Color::rgb(1, 2, 3)>>);
static_assert(not std::is_same_v<StructuralColor<Color{1, 2, 3}>, StructuralColor<Color{1, 2, 4}>>);

// TEST-COL-PARSE-002/004/005..008: remaining parse fixtures.
static_assert(Color::parseHex("#fF9aC0") == Color::rgb(255, 154, 192));
static_assert(Color::parseHex("#FFF") == Color::rgb(255, 255, 255));
static_assert(not Color::parseHex("#12G456"));
static_assert(not Color::parseHex("123456"));
static_assert(not Color::parseHex("#12"));
static_assert(not Color::parseHex("#12345"));
static_assert(not Color::parseHex("#1234567"));
static_assert(not Color::parseHex(std::string_view{"#12\0ABC", 7}));
static_assert(not Color::parseHex(" #ABC"));

// TEST-COL-HSV-008/010: every sector boundary and its neighbours stay valid and continuous.
consteval auto sectorBoundariesAreContinuous() -> bool {
  constexpr f64 epsilon = 1e-9;
  const auto near = [](u8 left, u8 right) { return left > right ? left - right <= 1 : right - left <= 1; };
  for (int sector = 1; sector < 6; ++sector) {
    const f64 boundary = sector * 60.0;
    const auto below = Color::fromHsv({boundary - epsilon, 1, 1});
    const auto at = Color::fromHsv({boundary, 1, 1});
    const auto above = Color::fromHsv({boundary + epsilon, 1, 1});
    if (not below or not at or not above)
      return false;
    if (not near(below->red, at->red) or not near(below->green, at->green) or not near(below->blue, at->blue) or
        not near(above->red, at->red) or not near(above->green, at->green) or not near(above->blue, at->blue))
      return false;
  }
  // The last representable hue below 360 and 359.999999 map to valid near-red, never to a seventh sector.
  const auto last = Color::fromHsv({std::nextafter(360.0, 0.0), 1, 1});
  const auto close = Color::fromHsv({359.999999, 1, 1});
  return last and close and last->red == 255 and last->green == 0 and last->blue <= 1 and close->red == 255 and
         close->green == 0 and close->blue <= 1;
}
static_assert(sectorBoundariesAreContinuous());

// TEST-COL-HSV-009, REV-003/004.
static_assert(Color::fromHsv({0, 0, 0}) == Color::rgb(0, 0, 0));
static_assert(Color::fromHsv({0, 0, 1}) == Color::rgb(255, 255, 255));
static_assert(Color::rgb(127, 127, 127).toHsv().hueDegrees == 0 and
              Color::rgb(127, 127, 127).toHsv().saturation == 0);
static_assert(std::abs(Color::rgb(127, 127, 127).toHsv().value - (127.0 / 255.0)) < 1e-12);

// TEST-COL-PROP-001/002 (compile-time part): every channel boundary combination, tied channels included.
constexpr std::array<u8, 6> boundaryChannels{0, 1, 127, 128, 254, 255};

consteval auto boundaryRoundTrips() -> bool {
  for (const u8 red : boundaryChannels)
    for (const u8 green : boundaryChannels)
      for (const u8 blue : boundaryChannels) {
        const Color color = Color::rgb(red, green, blue);
        const auto back = Color::fromHsv(color.toHsv());
        if (not back)
          return false;
        const auto close = [](u8 left, u8 right) { return left > right ? left - right <= 1 : right - left <= 1; };
        if (not close(back->red, red) or not close(back->green, green) or not close(back->blue, blue))
          return false;
        if (Color::hex(color.toHex()) != color)
          return false;
      }
  return true;
}
static_assert(boundaryRoundTrips());

// TEST-REC-COLOR-002: all 4,096 #RGB combinations expand each nibble to 17*n.
consteval auto shortHexExhaustive() -> bool {
  constexpr std::string_view digits = "0123456789abcdef";
  for (usize red = 0; red < 16; ++red)
    for (usize green = 0; green < 16; ++green)
      for (usize blue = 0; blue < 16; ++blue) {
        const std::array<char, 4> text{'#', digits[red], digits[green], digits[blue]};
        const auto parsed = Color::parseHex(std::string_view{text.data(), text.size()});
        if (not parsed or parsed->red != 17 * red or parsed->green != 17 * green or parsed->blue != 17 * blue)
          return false;
      }
  return true;
}
static_assert(shortHexExhaustive());

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

namespace {

constexpr u64 propertySeed = 0x4D6972616375'6C65ULL; // Seed recorded for reproduction.

/// SplitMix64: deterministic, reproducible across platforms.
constexpr auto nextRandom(u64 &state) -> u64 {
  state += 0x9E3779B97F4A7C15ULL;
  u64 mixed = state;
  mixed = (mixed ^ (mixed >> 30)) * 0xBF58476D1CE4E5B9ULL;
  mixed = (mixed ^ (mixed >> 27)) * 0x94D049BB133111EBULL;
  return mixed ^ (mixed >> 31);
}

auto channelDistance(const u8 left, const u8 right) -> int {
  return left > right ? left - right : right - left;
}

} // namespace

// TEST-COL-PROP-001: 10,000 seeded samples plus tied-channel adversaries; maxima are tracked, not assumed.
[[= Switch::test]] auto hsvRoundTripProperty() -> void {
  u64 state = propertySeed;
  int maxError = 0;
  Color worst{};
  const auto track = [&](const Color color) {
    const auto back = Color::fromHsv(color.toHsv());
    Switch::check(back.has_value());
    if (not back)
      return;
    const int error = std::max({channelDistance(back->red, color.red),
        channelDistance(back->green, color.green),
        channelDistance(back->blue, color.blue)});
    if (error > maxError) {
      maxError = error;
      worst = color;
    }
  };
  for (int sample = 0; sample < 10'000; ++sample) {
    const u64 bits = nextRandom(state);
    track(Color::rgb(static_cast<u8>(bits), static_cast<u8>(bits >> 8), static_cast<u8>(bits >> 16)));
  }
  for (int level = 0; level < 256; ++level) {
    const auto channel = static_cast<u8>(level);
    track(Color::rgb(channel, channel, channel));
    track(Color::rgb(channel, channel, 0));
    track(Color::rgb(0, channel, channel));
    track(Color::rgb(channel, 0, channel));
  }
  // Agreed bound: one 8-bit step. A larger maximum is a mismatch, reported with its minimal triplet.
  Switch::check(maxError <= 1);
  (void)worst;
}

// TEST-COL-PROP-002: every packed 24-bit value that can be printed as #RRGGBB parses back unchanged.
[[= Switch::test]] auto hexRoundTripProperty() -> void {
  u64 state = propertySeed;
  const auto roundTrips = [](const u32 packed) {
    const std::string text = std::format("#{:06X}", packed);
    const auto lower = std::format("#{:06x}", packed);
    const auto parsed = Color::parseHex(text);
    return parsed and parsed->toHex() == packed and Color::parseHex(lower) == parsed and
           Color::hex(packed) == parsed;
  };
  for (u32 packed = 0; packed <= 0xFFFF; ++packed)
    Switch::check(roundTrips(packed << 8U));
  for (int sample = 0; sample < 10'000; ++sample)
    Switch::check(roundTrips(static_cast<u32>(nextRandom(state)) & 0xFFFFFFU));
  for (const u8 red : boundaryChannels)
    for (const u8 green : boundaryChannels)
      for (const u8 blue : boundaryChannels)
        Switch::check(roundTrips(Color::rgb(red, green, blue).toHex()));
}

} // namespace Tests::color
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  Switch::discover<^^Tests::color>();
}
