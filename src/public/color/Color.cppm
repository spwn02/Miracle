export module Miracle.Color;

import std;

import Miracle.Numeric;

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

  /// Creates a Color from arbitrary integral channel values, or `nullopt` when any channel is outside 0..255.
  /// `bool` and floating-point inputs do not participate; each channel is range-checked before narrowing.
  template <std::integral R, std::integral G, std::integral B>
    requires(not std::same_as<std::remove_cv_t<R>, bool> and not std::same_as<std::remove_cv_t<G>, bool> and
             not std::same_as<std::remove_cv_t<B>, bool>)
  [[nodiscard]] static constexpr auto checkedRgb(const R red, const G green, const B blue) noexcept
      -> std::optional<Color> {
    if (not std::in_range<u8>(red) or not std::in_range<u8>(green) or not std::in_range<u8>(blue))
        [[unlikely]]
      return std::nullopt;

    return Color{.red = static_cast<u8>(red), .green = static_cast<u8>(green), .blue = static_cast<u8>(blue)};
  }

  /// Creates a Color from a 0xRRGGBB value; any bit above the 24-bit range yields `nullopt`.
  [[nodiscard]] static constexpr auto hex(const u32 rgb24) noexcept -> std::optional<Color> {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    if (rgb24 > 0xFFFFFF) [[unlikely]]
      return std::nullopt;

    return Color{.red = static_cast<u8>((rgb24 >> 16) & 0xFF),
        .green = static_cast<u8>((rgb24 >> 8) & 0xFF),
        .blue = static_cast<u8>(rgb24 & 0xFF)};
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }

  /// Parses exactly `#RRGGBB` or `#RGB` (case-insensitive ASCII hex digits). A short digit `n` expands to
  /// `17 * n`. A missing `#`, a `0x` prefix, whitespace, alpha or any other length yields `nullopt`.
  [[nodiscard]] static constexpr auto parseHex(const std::string_view text) noexcept -> std::optional<Color> {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    if ((text.size() != 4 and text.size() != 7) or text.front() != '#') [[unlikely]]
      return std::nullopt;

    const auto digit = [](const char character) noexcept -> std::optional<u8> {
      if (character >= '0' and character <= '9')
        return static_cast<u8>(character - '0');
      if (character >= 'a' and character <= 'f')
        return static_cast<u8>(character - 'a' + 10);
      if (character >= 'A' and character <= 'F')
        return static_cast<u8>(character - 'A' + 10);
      return std::nullopt;
    };

    const bool shorthand = text.size() == 4;
    std::array<u8, 3> channels{};
    for (usize index{}; index < channels.size(); ++index) {
      if (shorthand) {
        const auto nibble = digit(text[1 + index]);
        if (not nibble) [[unlikely]]
          return std::nullopt;
        channels[index] = static_cast<u8>(17 * *nibble);
      } else {
        const auto high = digit(text[1 + (2 * index)]);
        const auto low = digit(text[2 + (2 * index)]);
        if (not high or not low) [[unlikely]]
          return std::nullopt;
        channels[index] = static_cast<u8>((16 * *high) + *low);
      }
    }
    return Color{.red = channels[0], .green = channels[1], .blue = channels[2]};
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }

  /// Creates a Color from HSV, or `nullopt` for non-finite components, hue outside [0, 360) or saturation and
  /// value outside [0, 1]. Zero saturation yields gray; channels are quantized to nearest, halves up.
  [[nodiscard]] static constexpr auto fromHsv(const Hsv hsv) noexcept -> std::optional<Color> {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    const auto [hue, saturation, value] = hsv;
    if (not std::isfinite(hue) or not std::isfinite(saturation) or not std::isfinite(value) or hue < 0 or
        hue >= 360 or saturation < 0 or saturation > 1 or value < 0 or value > 1) [[unlikely]]
      return std::nullopt;

    const f64 chroma = value * saturation;
    const f64 scaledHue = hue / 60;
    const auto sector = static_cast<i32>(scaledHue);
    const f64 fraction = scaledHue - sector;
    const f64 second = chroma * (sector % 2 == 0 ? fraction : 1 - fraction);
    const f64 floor = value - chroma;

    f64 red{};
    f64 green{};
    f64 blue{};
    switch (sector) {
      case 0: red = chroma, green = second; break;
      case 1: red = second, green = chroma; break;
      case 2: green = chroma, blue = second; break;
      case 3: green = second, blue = chroma; break;
      case 4: red = second, blue = chroma; break;
      default: red = chroma, blue = second; break;
    }

    const auto quantize = [](const f64 channel) noexcept -> u8 {
      return static_cast<u8>(std::clamp(static_cast<i32>((channel * 255) + 0.5), 0, 255));
    };
    return Color{
        .red = quantize(red + floor), .green = quantize(green + floor), .blue = quantize(blue + floor)};
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
