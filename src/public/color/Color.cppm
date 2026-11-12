export module Miracle.Color;

import std;

import Miracle.Numeric;
import Miracle.Concepts;

export namespace Miracle {

/// Represents a color in the HSV color space.
struct Hsv final {
  /// The hue of the color in degrees, ranging from 0 to 360. For chromatic colors, this value represents the
  /// angle on the color wheel.
  f64 hueDegrees{};
  /// The saturation of the color, ranging from 0 to 1.
  f64 saturation{};
  /// The value (brightness) of the color, ranging from 0 to 1.
  f64 value{};

  [[nodiscard]] constexpr auto operator<=>(const Hsv &) const noexcept -> std::partial_ordering = default;
};

/// Represents a color in the RGB color space.
struct Color final {
  /// The red component of the color, ranging from 0 to 255.
  u8 red{};
  /// The green component of the color, ranging from 0 to 255.
  u8 green{};
  /// The blue component of the color, ranging from 0 to 255.
  u8 blue{};

  /// Creates a new Color instance from the specified red, green, and blue components.
  [[nodiscard]] static constexpr auto rgb(const u8 red, const u8 green, const u8 blue) noexcept -> Color {
    return Color{.red = red, .green = green, .blue = blue};
  }

  /// Creates a new Color instance from the specified red, green, and blue components, with bounds checking.
  [[nodiscard]] static constexpr auto checkedRgb(const Numeric auto red,
      const Numeric auto green,
      const Numeric auto blue) noexcept -> std::optional<Color> {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    if (not std::in_range<u8>(red) or not std::in_range<u8>(green) or not std::in_range<u8>(blue))
        [[unlikely]]
      return std::nullopt;

    return Color{.red = static_cast<u8>(red), .green = static_cast<u8>(green), .blue = static_cast<u8>(blue)};
  }

  /// Creates a new Color instance from the specified 24-bit RGB value.
  [[nodiscard]] static constexpr auto hex(const u32 rgb24) noexcept -> std::optional<Color> {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    if (rgb24 > 0xFFFFFF) [[unlikely]]
      return std::nullopt;

    return Color{.red = static_cast<u8>((rgb24 >> 16) & 0xFF),
        .green = static_cast<u8>((rgb24 >> 8) & 0xFF),
        .blue = static_cast<u8>(rgb24 & 0xFF)};
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }

  /// Creates a new Color instance from the specified hexadecimal color string (e.g., "#RRGGBB" or "#RGB").
  [[nodiscard]] static constexpr auto parseHex(std::string_view text) noexcept -> std::optional<Color> {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    if (text.empty()) [[unlikely]]
      return std::nullopt;

    if (text.front() == '#')
      text.remove_prefix(1);

    if (text.length() != 3 and text.length() != 6) [[unlikely]]
      return std::nullopt;

    u32 num{};

    const auto res = std::from_chars(text.begin(), text.end(), num, 16);
    if (not res) [[unlikely]]
      return std::nullopt;

    if (text.length() == 3)
      num |= (num << 12);

    return hex(num);
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }

  /// Creates a new Color instance from the specified HSV color.
  [[nodiscard]] static constexpr auto fromHsv(const Hsv hsv) noexcept -> std::optional<Color> {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    if (not std::isfinite(hsv.hueDegrees) or not std::isfinite(hsv.saturation) or
        not std::isfinite(hsv.value) or (hsv.hueDegrees < 0 or hsv.hueDegrees >= 360) or
        (hsv.saturation < 0 or hsv.saturation > 1) or (hsv.value < 0 or hsv.value > 1)) [[unlikely]]
      return std::nullopt;

    const auto func = [=](u8 n) -> f64 {
      const f64 chroma = std::fmod(n + (hsv.hueDegrees / 60), 6);
      return hsv.value -
             (hsv.value * hsv.saturation * std::max(f64{0.0}, std::min({chroma, 4 - chroma, f64{1.0}})));
    };
    return Color::rgb(static_cast<u8>(std::floor((func(5) * 255) + 0.5)),
        static_cast<u8>(std::floor((func(3) * 255) + 0.5)),
        static_cast<u8>(std::floor((func(1) * 255) + 0.5)));
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }

  /// Converts the current Color instance to a 24-bit RGB value.
  [[nodiscard]] constexpr auto toHex() const noexcept -> u32 {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    return (static_cast<u32>(red) << 16) | (static_cast<u32>(green) << 8) | blue;
  }

  /// Converts the current Color instance to its corresponding HSV representation.
  [[nodiscard]] constexpr auto toHsv() const noexcept -> Hsv {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    const f64 r{red / 255.0};   // NOLINT(readability-identifier-length)
    const f64 g{green / 255.0}; // NOLINT(readability-identifier-length)
    const f64 b{blue / 255.0};  // NOLINT(readability-identifier-length)

    const f64 cmax = std::max({r, g, b});
    const f64 delta = cmax - std::min({r, g, b});
    f64 hueDegrees{};

    if (delta != 0) {
      if (cmax == r)
        hueDegrees = 60 * std::fmod(((g - b) / delta) + 6, 6);
      else if (cmax == g)
        hueDegrees = 60 * (((b - r) / delta) + 2);
      else
        hueDegrees = 60 * (((r - g) / delta) + 4);
    }

    if (hueDegrees < 0)
      hueDegrees += 360;
    else if (hueDegrees >= 360)
      hueDegrees -= 360;

    return {
        .hueDegrees = hueDegrees,
        .saturation = cmax == 0 ? 0 : (delta / cmax),
        .value = cmax,
    };
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }

  [[nodiscard]] constexpr auto operator<=>(const Color &) const noexcept -> std::strong_ordering = default;
};

} // namespace Miracle

using namespace Miracle;

template <>
struct std::formatter<Hsv, char> : std::formatter<std::string_view, char> {
  bool pretty{};

  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) -> ParseContext::iterator {
    auto iter = ctx.begin();

    if (iter == ctx.end())
      return iter;

    if (*iter == '#') {
      pretty = true;
      ++iter;
    }

    if (iter != ctx.end() && *iter != '}')
      throw std::format_error("Invalid format args for Hsv.");

    return iter;
  }

  constexpr auto format(const Hsv &hsv, std::format_context &ctx) const -> std::format_context::iterator {
    return pretty ? std::formatter<std::string_view, char>::format(
                        std::format("Hsv {{\n  hueDegrees: {},\n  saturation: {},\n  value: {}\n}}",
                            hsv.hueDegrees,
                            hsv.saturation,
                            hsv.value),
                        ctx)
                  : std::formatter<std::string_view, char>::format(
                        std::format("Hsv {{ hueDegrees: {}, saturation: {}, value: {} }}",
                            hsv.hueDegrees,
                            hsv.saturation,
                            hsv.value),
                        ctx);
  }
};

template <>
struct std::formatter<Color, char> : std::formatter<std::string_view, char> {
  bool pretty{};

  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) -> ParseContext::iterator {
    auto iter = ctx.begin();

    if (iter == ctx.end())
      return iter;

    if (*iter == '#') {
      pretty = true;
      ++iter;
    }

    if (iter != ctx.end() && *iter != '}')
      throw std::format_error("Invalid format args for Color.");

    return iter;
  }

  constexpr auto format(const Color &hsv, std::format_context &ctx) const -> std::format_context::iterator {
    return pretty
               ? std::formatter<std::string_view, char>::format(
                     std::format(
                         "Color {{\n  red: {},\n  green: {},\n  blue: {}\n}}", hsv.red, hsv.green, hsv.blue),
                     ctx)
               : std::formatter<std::string_view, char>::format(
                     std::format("Color {{ red: {}, green: {}, blue: {} }}", hsv.red, hsv.green, hsv.blue),
                     ctx);
  }
};
